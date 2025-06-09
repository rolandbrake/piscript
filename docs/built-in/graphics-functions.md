# 🎨 Graphics Functions

### pixel(x, y, color)

**Description:**  
Sets a pixel at position `(x, y)` to the specified `color`.  
This function **does not immediately render** the pixel — changes are only visible after calling `draw()`.

**Screen Details:**
- Resolution: `128 × 128` pixels
- Coordinate system:
  - `x` ranges from `0` to `127`
  - `y` ranges from `0` to `127`
- Color is an integer in the range `0–31`, representing a color in the 32-color palette.

**Arguments:**
- `x` *(int)* – The horizontal position (0 ≤ x < 128)
- `y` *(int)* – The vertical position (0 ≤ y < 128)
- `color` *(int)* – The color index (0–31)

**Behavior:**
- Has **no visual effect** until `draw()` is called.
- Coordinates outside the screen bounds are ignored or clamped (depending on implementation).
- Pixels persist between frames unless explicitly cleared or overwritten.

**Examples:**
```piscript
pixel(10, 20, 7)   // Set pixel at (10, 20) to color 7
pixel(64, 64, 15)  // Set center pixel to color 15
draw()                // Apply all pixel changes to the screen
```
---

### line(x0, y0, x1, y1, color)

**Description:**  
Draws a straight line from point `(x0, y0)` to point `(x1, y1)` in the specified `color`.  
The line is drawn on a **128×128** pixel screen but only appears after calling `draw()`.

**Arguments:**
- `x0` *(int)* – Starting x-coordinate (0 ≤ x < 128)
- `y0` *(int)* – Starting y-coordinate (0 ≤ y < 128)
- `x1` *(int)* – Ending x-coordinate (0 ≤ x < 128)
- `y1` *(int)* – Ending y-coordinate (0 ≤ y < 128)
- `color` *(int)* – Color index (0–31)

**Behavior:**
- Uses **Bresenham's line algorithm** (or similar) to determine which pixels to set.
- Drawing is deferred until `draw()` is called.
- Coordinates outside the screen bounds are either ignored or clipped.
- The line is 1 pixel thick and solid by default.

**Examples:**
```piscript
line(0, 0, 127, 127, 8)      // Draws a diagonal line across the screen
line(10, 20, 100, 20, 12)    // Draws a horizontal line
line(64, 0, 64, 127, 3)      // Draws a vertical line down the center
draw()                          // Renders all line changes to the screen
```
---

### draw()

**Description:**  
Renders all pending pixel changes to the screen.  
This function **must be called** to make the results of drawing functions like `pixel()`, `line()`, `rect()`, etc., visible on the **128×128** pixel display.

**Behavior:**
- Applies all drawing operations accumulated since the last `draw()` call.
- Works like a **frame update** or **screen refresh**.
- If `draw()` is not called, the screen will not visually change, even if drawing commands were issued.
- Intended to be called **once per frame** for optimal performance and consistency.

**Typical Use Pattern:**
```piscript
// Frame start
pixel(10, 10, 4)
line(0, 0, 127, 127, 7)
rect(30, 30, 60, 40, 15, true)
draw()  // Flush all changes to the screen
```
---

### clear(color)

**Description:**  
Clears the entire **128×128** pixel screen by filling it with the specified `color`.  
This sets every pixel to the same color and **overwrites all previous drawing** commands in the current frame.

**Arguments:**
- `color` *(int)* – A value from `0` to `31`, representing the background fill color (from the 32-color palette).

**Behavior:**
- Must be followed by `draw()` to visually apply the clear operation.
- Typically used at the beginning of a frame to reset the screen before drawing new elements.
- Efficiently sets the screen to a uniform color.

**Examples:**
```piscript
clear(0)         // Clears the screen to color index 0 (often black)
pixel(64, 64, 15)
draw()        // Screen shows a black background with one pixel at center

clear(7)         // Clear the screen to color 7 (e.g. gray background)
draw()
```
---

### circ(x, y, r, color, filled)

**Description:**  
Draws the outline of a **circle** with center at `(x, y)`, radius `r`, and the specified `color` on a **128×128** pixel screen.  
The circle is only visible after calling `draw()`.

**Arguments:**
- `x` *(int)* – X-coordinate of the center (0 ≤ x < 128)
- `y` *(int)* – Y-coordinate of the center (0 ≤ y < 128)
- `r` *(int)* – Radius of the circle (positive integer)
- `color` *(int)* – Color index (0–31) from the palette
- `filled` *(bool)* - draw filled shape if true  

**Behavior:**
- Draws only the **outline** of the circle (1-pixel thick).
- Uses a pixel-based circle algorithm like **Midpoint Circle Algorithm**.
- Pixels outside the screen bounds are ignored or clipped.
- Drawing is deferred until `draw()` is called.

**Examples:**
```piscript
circ(64, 64, 30, 10)     // Draws a circle centered in the screen with radius 30
circ(10, 10, 5, 4)       // Draws a small circle near the top-left
draw()                   // Renders all circle outlines to the screen
```
---

### rect(x, y, w, h, color, [filled])

**Description:**  
Draws the **outline** of a rectangle with the top-left corner at `(x, y)`, width `w`, height `h`, and the specified `color` on a **128×128** pixel screen.  
The rectangle will appear **only after** calling `draw()`.

**Arguments:**
- `x` *(int)* – X-coordinate of the top-left corner (0 ≤ x < 128)
- `y` *(int)* – Y-coordinate of the top-left corner (0 ≤ y < 128)
- `w` *(int)* – Width of the rectangle (positive integer)
- `h` *(int)* – Height of the rectangle (positive integer)
- `color` *(int)* – Color index (0–31) from the palette
- `filled` *(bool)* - draw filled shape if true 

**Behavior:**
- Draws only the **border** (outline) of the rectangle.
- Uses thin 1-pixel lines for edges.
- Pixels outside the screen boundaries are ignored or clipped.
- No pixels will be drawn if `w` or `h` is `0` or negative.
- Must call `draw()` to render the shape.

**Examples:**
```piscript
rect(10, 10, 50, 30, 6)     // Draws a rectangle at (10,10) with width 50 and height 30
rect(0, 0, 128, 128, 12)    // Draws a border around the full screen
draw()                   // Displays all rectangles drawn
```
---

### poly(points, color, filled)

**Description:**  
Draws a **polygon** on the **128×128** pixel screen using the provided list of point coordinates.  
The polygon can be either **filled** or just an **outline** based on the `is_filled` flag.  
Changes appear only after calling `draw()`.

**Arguments:**
- `points` *(list of ints)* – A flat list of coordinates in the format `[x0, y0, x1, y1, ..., xn, yn]`. Must contain an even number of values (pairs).
- `color` *(int)* – Color index (0–31) from the palette.
- `filled` *(bool)* – If `true`, draws a filled polygon; if `false`, draws an outlined polygon.

**Behavior:**
- The polygon connects all given points in order, and automatically closes the shape by linking the last point back to the first.
- Coordinates outside the screen bounds are clipped.
- Does nothing if the `points` list is invalid (e.g., not even number of elements or fewer than 6 values).
- The polygon is not rendered until `draw()` is called.

**Examples:**
```piscript
// Draw a filled triangle
poly([0, 0, 10, 0, 0, 30], 6, true)
draw()

// Draw an outlined diamond

poly([64, 30, 90, 64, 64, 98, 38, 64], 6, false)
draw()
```
---

### sprite(id, x, y)

**Description:**  
Draws a **sprite** from the sprite storage onto the **128×128** screen at position `(x, y)`.  
The sprite must be pre-defined and stored using its `id`.  
Rendering is deferred until `draw()` is called.

**Arguments:**
- `id` *(int)* – The unique ID/index of the sprite in the sprite storage.
- `x` *(int)* – The x-coordinate of where to place the top-left corner of the sprite (0 ≤ x < 128).
- `y` *(int)* – The y-coordinate of where to place the top-left corner of the sprite (0 ≤ y < 128).

**Behavior:**
- Draws the sprite located in memory under the given `id`.
- Sprites are typically 8×8 or 16×16 pixels depending on your implementation.
- Sprites drawn off-screen are clipped.
- Requires a call to `draw()` to update the visible screen.

**Examples:**
```piscript
sprite(0, 10, 20)     // Draw sprite with ID 0 at position (10, 20)
sprite(5, 64, 64)     // Draw sprite with ID 5 at center of the screen
draw()             // Renders all sprite drawings to the screen
```
---
### color(x, y)

**Description:**  
Returns the **color index** of the pixel at the coordinates `(x, y)` on the **128×128** screen.

**Arguments:**
- `x` *(int)* – The x-coordinate of the pixel (0 ≤ x < 128)
- `y` *(int)* – The y-coordinate of the pixel (0 ≤ y < 128)

**Returns:**
- *(int)* – A value between `0` and `31`, representing the color index at the given position.

**Behavior:**
- This function reads the color of a pixel that has been **drawn and committed** via `draw()`.
- If the pixel lies outside the screen bounds, the return value is implementation-defined (typically `-1` or `null`).
- Useful for **collision detection**, **sampling screen content**, or **interactive effects**.

**Examples:**
```piscript
clear(0)
pixel(5, 5, 10)
draw()

c = color(5, 5)       // c = 10
print(c)
```

---