# Time Functions

These functions provide basic time-related utilities, such as delaying execution or retrieving the current time.

---

### sleep(milliseconds)

**Description:**  
Pauses execution of the program for the specified number of `milliseconds`.

**Arguments:**
- `milliseconds` *(number)* – The number of milliseconds to wait

**Returns:**
- *(none)* – This function does not return a value.

**Behavior:**
- Freezes the execution of the script for the given duration.
- Useful for timing effects, animations, or delaying logic.

**Examples:**
```piscript
print("Wait for 2 seconds...")
sleep(2000)
print("Done!")

```
---

### `time()`

**Description:**  
Returns the current time in **seconds** since the start of the program.

**Arguments:**
- *(none)* – This function takes no arguments.

**Returns:**
- *(number)* – The number of seconds (as a float) since the program began running.

**Behavior:**
- Measures runtime duration, not real-world time (i.e. it's relative, not absolute).
- Typically used for animations, measuring elapsed time, or custom timers.

**Examples:**
```piscript
let start = time()
sleep(2000)
let elapsed = time() - start
print("Elapsed time: " + elapsed)  // Output: Elapsed time: 2
```
---
