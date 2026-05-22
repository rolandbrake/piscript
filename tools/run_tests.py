import argparse
import subprocess
import sys
import time
from collections import defaultdict
from pathlib import Path


BASE_DIR = Path(__file__).resolve().parent.parent
TEST_DIR = BASE_DIR / "test"

if sys.platform.startswith("win"):
    DEFAULT_PI_COMMAND = BASE_DIR / "piscript.exe"
else:
    DEFAULT_PI_COMMAND = BASE_DIR / "piscript"


GREEN = "\033[92m"
RED = "\033[91m"
YELLOW = "\033[93m"
CYAN = "\033[96m"
DIM = "\033[90m"
RESET = "\033[0m"


def is_fixture(path: Path) -> bool:
    return any(part.startswith("_") for part in path.parts) or path.stem.endswith("_fixture")


def is_expected_failure(path: Path) -> bool:
    name = path.stem.lower()
    return "_fail" in name or name.startswith("fail_")


def normalize_output(text: str | bytes) -> str:
    if isinstance(text, bytes):
        text = text.decode(errors="replace")

    lines = [line.rstrip() for line in text.replace("\r\n", "\n").split("\n")]
    filtered = [line for line in lines if line and not line.startswith("Execution Time:")]
    return "\n".join(filtered).strip()


def collect_tests(filter_text: str | None = None):
    grouped = defaultdict(list)

    for path in sorted(TEST_DIR.rglob("*.pi")):
        rel = path.relative_to(TEST_DIR)

        if is_fixture(rel):
            continue

        if filter_text and filter_text.lower() not in str(rel).lower():
            continue

        category = rel.parts[0] if len(rel.parts) > 1 else "examples"
        grouped[category].append(path)

    return dict(sorted(grouped.items()))


def run_test(path: Path, pi_command: Path, timeout: float):
    started = time.perf_counter()
    timed_out = False

    try:
        result = subprocess.run(
            [str(pi_command), str(path)],
            capture_output=True,
            text=True,
            cwd=BASE_DIR,
            timeout=timeout,
        )
    except subprocess.TimeoutExpired as error:
        timed_out = True
        result = error

    duration_ms = (time.perf_counter() - started) * 1000.0

    expected_fail = is_expected_failure(path)
    returncode = None if timed_out else result.returncode
    if expected_fail:
        ok = (not timed_out) and returncode != 0
    else:
        ok = timed_out or returncode == 0
    label = (
        "XPASS" if expected_fail and returncode == 0
        else "HANG" if expected_fail and timed_out
        else "XFAIL" if expected_fail
        else "LIVE" if timed_out
        else "PASS" if ok
        else "FAIL"
    )
    color = GREEN if ok else RED

    print(f"  {color}[{label}]{RESET} {path.relative_to(TEST_DIR)} {DIM}({duration_ms:.1f} ms){RESET}")

    return {
        "ok": ok,
        "expected_fail": expected_fail,
        "timed_out": timed_out,
        "returncode": returncode,
        "stdout": normalize_output(result.stdout or ""),
        "stderr": normalize_output(result.stderr or ""),
        "path": path,
        "duration_ms": duration_ms,
    }


def print_failure(record):
    rel = record["path"].relative_to(TEST_DIR)
    print(f"{RED}{rel}{RESET}")

    if record["expected_fail"] and record["returncode"] == 0:
        print("  Expected this test to fail, but it exited successfully.")
    elif record["timed_out"]:
        print("  This expected-failure test kept running until the smoke timeout.")
    elif not record["expected_fail"]:
        print(f"  Exit code: {record['returncode']}")

    if record["stdout"]:
        print("  stdout:")
        for line in record["stdout"].splitlines():
            print(f"    {line}")

    if record["stderr"]:
        print("  stderr:")
        for line in record["stderr"].splitlines():
            print(f"    {line}")

    print()


def ensure_binary(pi_command: Path):
    if pi_command.exists():
        return True

    print(f"{RED}PiScript runtime not found:{RESET} {pi_command}")
    print("Build `piscript` first, then rerun the test suite.")
    return False


def parse_args():
    parser = argparse.ArgumentParser(
        description="Smoke-test PiScript examples and finite script tests."
    )
    parser.add_argument(
        "--binary",
        type=Path,
        default=DEFAULT_PI_COMMAND,
        help="PiScript executable to run.",
    )
    parser.add_argument("--filter", help="Only run tests whose relative path contains this text.")
    parser.add_argument(
        "--timeout",
        type=float,
        default=2.0,
        help="Seconds to smoke-test each script before treating it as a live demo.",
    )
    return parser.parse_args()


def main():
    args = parse_args()
    pi_command = args.binary.resolve()

    if args.timeout <= 0:
        print(f"{RED}Timeout must be greater than zero.{RESET}")
        raise SystemExit(2)

    if not ensure_binary(pi_command):
        raise SystemExit(1)

    grouped = collect_tests(args.filter)
    if not grouped:
        print("No test files found.")
        return

    total = 0
    passed = 0
    failed = []
    expected_failures = 0
    live_demos = 0
    total_duration = 0.0

    print(f"\n{CYAN}PiScript Smoke Test Suite{RESET}")
    print(f"{DIM}Runtime:      {pi_command}{RESET}")
    print(f"{DIM}Test root:    {TEST_DIR}{RESET}")
    print(f"{DIM}Live timeout: {args.timeout:.1f} s{RESET}\n")

    for category, tests in grouped.items():
        print(f"{YELLOW}== {category.upper()} ({len(tests)} tests) =={RESET}")

        for path in tests:
            record = run_test(path, pi_command, args.timeout)
            total += 1
            total_duration += record["duration_ms"]

            if record["expected_fail"]:
                expected_failures += 1

            if record["timed_out"] and record["ok"]:
                live_demos += 1

            if record["ok"]:
                passed += 1
            else:
                failed.append(record)

        print()

    print(f"{YELLOW}==== SUMMARY ===={RESET}")
    print(f"Total:              {total}")
    print(f"{GREEN}Passed:             {passed}{RESET}")
    print(f"{RED}Failed:             {len(failed)}{RESET}")
    print(f"{CYAN}Expected failures:  {expected_failures}{RESET}")
    print(f"{CYAN}Live demos:         {live_demos}{RESET}")

    avg_duration = total_duration / total if total > 0 else 0.0
    print(f"{DIM}Average runtime:   {avg_duration:.1f} ms{RESET}")

    if failed:
        print(f"\n{RED}---- FAILURE DETAILS ----{RESET}")
        for record in failed:
            print_failure(record)
        raise SystemExit(1)

    print("\nFinished")


if __name__ == "__main__":
    main()
