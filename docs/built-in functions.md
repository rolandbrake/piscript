# PiScript Built-in Functions

PiScript comes with a collection of powerful built-in functions designed to simplify common tasks in programming such as input/output handling, collection manipulation, mathematical operations, and utility tasks.

These functions are automatically available without requiring imports and are grouped into the following categories:

- **I/O Functions**: For interacting with users or the environment (e.g. `print`, `input`)
- **Collection Functions**: For working with lists and ranges (e.g. `len`, `map`, `filter`)
- **Math Functions**: For basic and advanced numeric calculations (e.g. `abs`, `sqrt`)
- **Utility Functions**: Miscellaneous helpers to improve code clarity or manage behavior (e.g. `type`, `range`)

---

## Why Use Built-in Functions?

Built-in functions are optimized and tested for reliability and performance. Using them makes your code:

- More concise
- Easier to read and maintain
- Cross-compatible with the PiScript interpreter

---

## Usage Example

```piscript
nums = [1, 2, 3, 4, 5]
squared = map(nums, (x) -> x * x)
println(squared)  // expect: [1, 4, 9, 16, 25]
