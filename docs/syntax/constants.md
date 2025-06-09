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

---

**Note:** These constants are read-only. Attempting to assign to them may result in an error or be ignored by the interpreter.

```piscript
print("Screen size: " + WIDTH + "x" + HEIGHT);
let area = PI * 10 * 10;
print("Circle area with r = 10: " + area);
