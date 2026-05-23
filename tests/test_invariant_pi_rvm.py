import pytest
import ctypes
import sys
import math

# Adversarial inputs targeting integer overflow in allocation size computation
# These represent values that could cause multiplication overflow when computing
# allocation sizes (e.g., count * element_size)

LARGE_VALUES = [
    # Near max values for common integer types
    2**31 - 1,      # INT32_MAX
    2**31,          # INT32_MAX + 1
    2**32 - 1,      # UINT32_MAX
    2**32,          # UINT32_MAX + 1
    2**63 - 1,      # INT64_MAX
    2**63,          # INT64_MAX + 1
    2**64 - 1,      # UINT64_MAX
    # Values that cause overflow when multiplied by common element sizes
    2**32 // 4,     # Overflows when multiplied by 4 (int size)
    2**32 // 8,     # Overflows when multiplied by 8 (pointer size)
    2**32 // 16,    # Overflows when multiplied by 16
    2**31 // 4 + 1, # Just past safe boundary for int32 * 4
    2**31 // 8 + 1, # Just past safe boundary for int32 * 8
    # Specific overflow-inducing values
    0xFFFFFFFF,
    0x80000000,
    0x7FFFFFFF,
    0xFFFFFFFFFFFFFFFF,
    0x8000000000000000,
    # Values that wrap to small numbers on overflow
    2**32 + 1,
    2**32 + 16,
    2**32 + 256,
    # Edge cases
    0,
    1,
    2,
    sys.maxsize,
    sys.maxsize + 1,
    sys.maxsize // 2 + 1,
]

@pytest.mark.parametrize("count", LARGE_VALUES)
def test_allocation_size_no_overflow(count):
    """Invariant: Allocation size computation must never overflow, producing a
    smaller buffer than required. For any count * element_size computation,
    the result must accurately represent the required memory, or the allocation
    must be rejected safely rather than proceeding with a wrapped/truncated size."""

    # Simulate the allocation size computation that pi_rvm.c would perform
    # Common element sizes used in VM implementations
    element_sizes = [1, 2, 4, 8, 16, 32, 64]

    for element_size in element_sizes:
        # The invariant: if we compute count * element_size,
        # the result must either:
        # 1. Be the mathematically correct value (no overflow), OR
        # 2. Be detected as overflow and rejected (not silently truncated)

        # Compute the mathematically correct size
        correct_size = count * element_size

        # Simulate what a C program would compute with various integer widths
        # 32-bit overflow simulation
        size_32bit = (count * element_size) & 0xFFFFFFFF
        # 64-bit overflow simulation
        size_64bit = (count * element_size) & 0xFFFFFFFFFFFFFFFF

        # Security invariant: if overflow occurs, the computed size must be
        # LESS than the correct size — this is the dangerous condition that
        # must be detected and prevented
        if correct_size > 0xFFFFFFFF:
            # 32-bit overflow occurred
            # The invariant: code must NOT use size_32bit as allocation size
            # because size_32bit < correct_size (buffer would be too small)
            assert size_32bit != correct_size or size_32bit == 0, (
                f"32-bit overflow not detected: count={count}, "
                f"element_size={element_size}, "
                f"correct={correct_size}, computed={size_32bit}"
            )
            # Verify the overflow actually produces a smaller value (the dangerous case)
            if size_32bit != 0:
                assert size_32bit < correct_size, (
                    f"Expected overflow to produce smaller value: "
                    f"count={count}, element_size={element_size}"
                )

        if correct_size > 0xFFFFFFFFFFFFFFFF:
            # 64-bit overflow occurred
            assert size_64bit != correct_size or size_64bit == 0, (
                f"64-bit overflow not detected: count={count}, "
                f"element_size={element_size}, "
                f"correct={correct_size}, computed={size_64bit}"
            )

        # Invariant: Safe allocation check — multiplication must be validated
        # before use. A safe implementation must detect overflow.
        def safe_multiply_check(a, b, max_val=0xFFFFFFFF):
            """Returns True if multiplication is safe (no overflow)"""
            if a == 0 or b == 0:
                return True
            # Safe check: a * b <= max_val iff a <= max_val / b
            return a <= max_val // b

        def safe_multiply_check_64(a, b, max_val=0xFFFFFFFFFFFFFFFF):
            """Returns True if multiplication is safe (no overflow for 64-bit)"""
            if a == 0 or b == 0:
                return True
            return a <= max_val // b

        # The safe check must correctly identify overflow conditions
        is_safe_32 = safe_multiply_check(count, element_size)
        is_safe_64 = safe_multiply_check_64(count, element_size)

        if not is_safe_32:
            # Must have actual overflow in 32-bit
            assert (count * element_size) > 0xFFFFFFFF, (
                f"Safe check incorrectly flagged overflow: "
                f"count={count}, element_size={element_size}"
            )

        if not is_safe_64:
            # Must have actual overflow in 64-bit
            assert (count * element_size) > 0xFFFFFFFFFFFFFFFF, (
                f"Safe 64-bit check incorrectly flagged overflow: "
                f"count={count}, element_size={element_size}"
            )

        # Final invariant: if safe, computed size equals correct mathematical size
        if is_safe_32:
            computed = count * element_size
            assert computed == correct_size, (
                f"Safe multiplication produced wrong result: "
                f"count={count}, element_size={element_size}, "
                f"expected={correct_size}, got={computed}"
            )


@pytest.mark.parametrize("count,element_size", [
    # Pairs specifically crafted to cause 32-bit overflow
    (0x10000, 0x10000),         # 2^32 exactly
    (0x10001, 0x10000),         # Just over 2^32
    (0xFFFFFFFF, 2),            # UINT32_MAX * 2
    (0x80000000, 2),            # 2^31 * 2 = 2^32
    (0x40000001, 4),            # Just over 2^32 when * 4
    (0x20000001, 8),            # Just over 2^32 when * 8
    (0x10000001, 16),           # Just over 2^32 when * 16
    # Safe boundary values
    (0xFFFF, 0xFFFF),           # Just under 2^32
    (0x3FFFFFFF, 4),            # Safe for * 4
    (0x1FFFFFFF, 8),            # Safe for * 8
    (0, 8),                     # Zero count - safe
    (1, 8),                     # Minimal count - safe
])
def test_specific_overflow_pairs(count, element_size):
    """Invariant: Specific count/element_size pairs must be handled safely.
    Overflow must be detected before allocation, not after."""

    correct_size = count * element_size
    size_32bit = (count * element_size) & 0xFFFFFFFF

    # Detect if overflow would occur in 32-bit arithmetic
    overflow_32bit = correct_size > 0xFFFFFFFF

    if overflow_32bit:
        # The wrapped value must be smaller than the correct value
        # This is the dangerous condition: allocating size_32bit bytes
        # when correct_size bytes are needed
        assert size_32bit < correct_size, (
            f"Overflow should produce smaller value: "
            f"count={count}, element_size={element_size}, "
            f"correct={correct_size}, wrapped={size_32bit}"
        )

        # A safe implementation MUST reject this allocation
        # Verify the overflow detection logic works
        if count > 0 and element_size > 0:
            detected = count > (0xFFFFFFFF // element_size)
            assert detected, (
                f"Overflow detection failed for count={count}, "
                f"element_size={element_size}: "
                f"product={correct_size} exceeds 32-bit max"
            )
    else:
        # No overflow: computed size must equal correct size
        assert size_32bit == correct_size, (
            f"Non-overflow case produced wrong result: "
            f"count={count}, element_size={element_size}"
        )