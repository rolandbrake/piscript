# System Functions

System functions provide information about the runtime environment, such as performance metrics.

---

### fps()

**Description:**  
Returns the current **frames per second (FPS)** being rendered by the program.

**Arguments:**
- *(none)* – This function does not take any parameters.

**Returns:**
- *(number)* – A floating-point number representing the current frame rate.

**Behavior:**
- Measures how many times the screen is being redrawn per second.
- Useful for performance monitoring, animations, and debugging rendering speed.
- The returned value may fluctuate depending on processing load.

**Examples:**
```piscript
print("FPS: " + fps())  // Example output: FPS: 60.0
```
---

### error(message)

**Description:**  
Immediately stops the program and reports an error with the given message.

**Arguments:**
- `message` *(string)* – A description of the error to display.

**Returns:**
- *(none)* – This function does not return. It halts execution.

**Behavior:**
- Displays the provided error message to the user.
- Terminates the running program immediately.
- Useful for debugging or signaling critical failures.

**Examples:**
```piscript
let x = -1;
if (x < 0) {
    error("Negative value not allowed");
}
```
---

### cursor(x, y)

**Description:**  
Sets the **text cursor position** on the screen for subsequent `print()` calls.

**Arguments:**
- `x` *(number)* – The x-coordinate (horizontal) in pixels, from `0` to `127`.
- `y` *(number)* – The y-coordinate (vertical) in pixels, from `0` to `127`.

**Returns:**
- *(none)* – This function does not return a value.

**Behavior:**
- Moves the cursor to the specified position on the 128×128 pixel screen.
- Affects where the next text printed with `print()` will appear.
- Does **not** draw anything by itself—only sets the print location.

**Examples:**
```piscript
cursor(10, 20);
print("Hello, world!");

cursor(50, 100);
print("Second line");
```
---

### mouse()

**Description:**  
Returns the current **mouse position** on the screen.

**Arguments:**
- *(none)* – This function takes no arguments.

**Returns:**
- *(list)* – A list of two numbers: `[x, y]` representing the current mouse coordinates on the 128×128 screen.

**Behavior:**
- Tracks the mouse position relative to the top-left corner of the screen (`0, 0`).
- The values range from `0` to `127` for both `x` and `y`.
- If the mouse is outside the canvas, it may return the last known position or clamp to bounds, depending on implementation.

**Examples:**
```piscript
let pos = mouse()
print("Mouse X: " + pos[0])
print("Mouse Y: " + pos[1])

// Example: draw a dot where the mouse is
pi_pixel(pos[0], pos[1], 8)
draw()
```
---

### zen()

**Description:**  
Prints a list of guiding principles or philosophies behind **Piscript** — inspired by the spirit of simplicity, creativity, and minimalism.

**Arguments:**
- *(none)* – This function takes no arguments.

**Returns:**
- *(none)* – It simply prints to the console or screen.

**Behavior:**
- Displays a predefined set of core ideas or values that Piscript encourages.
- Useful for inspiration, teaching, or understanding the language's philosophy.

**Example:**
```piscript
zen()
```
---


This list will be updated with future additions to PiScript.

---

More built-in functions will be added as PiScript evolves. Contributions and suggestions are welcome!
