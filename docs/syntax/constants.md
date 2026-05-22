## 📌 Predefined Constants

Piscript includes several built-in constants for convenience. These constants are always available and do not need to be defined by the user.

---

### 🔢 Math Constants

| Constant | Description               | Example        |
|----------|---------------------------|----------------|
| `PI`     | The mathematical π ≈ 3.14159 | `print(PI);`|
| `E`      | Euler’s number ≈ 2.71828   | `print(E);`   |

---

### 🎨 Graphics Constants

| Constant   | Description                           | Example           |
|------------|---------------------------------------|-------------------|
| `WIDTH`    | The width of the screen (128 pixels)  | `print(WIDTH);`   |
| `HEIGHT`   | The height of the screen (128 pixels) | `print(HEIGHT);`  |

PiScript exposes the default palette as named color constants. The base palette
uses names such as `COLOR_SOFT_BLACK`, `COLOR_SKY_BLUE`,
`COLOR_NEON_GREEN`, and `COLOR_SOFT_IVORY`. The themed palette groups use
prefixes such as `COLOR_GB_`, `COLOR_NES_`, and `COLOR_SEGA_`.

```piscript
clear(COLOR_SOFT_BLACK)
rect(8, 8, 32, 20, COLOR_NEON_GREEN, true)
pixel(64, 64, 0xff00ffff) // opaque custom cyan in 0xAARRGGBB form
pixel(65, 64, 0x8000ffff) // same color with alpha blending
draw()
```

Values from `0` to `79` remain palette indices for compatibility. Larger color
numbers are read as packed `0xAARRGGBB` colors.

---

**Note:** These constants are read-only. Attempting to assign to them may result in an error or be ignored by the interpreter.

```piscript
print("Screen size: " + WIDTH + "x" + HEIGHT);
let area = PI * 10 * 10;
print("Circle area with r = 10: " + area);
