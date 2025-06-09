# ⌨️ I/O Functions

### println(value1, value2, ...)

Prints values to the standard output followed by a newline.

- **Parameters:**

  - `value1, value2, ...` – One or more values to print.

- **Returns:**

  - `null`

- **Behavior:**

  - Concatenates all values as strings with spaces and prints them.
  - Automatically appends a newline at the end.

- **Examples:**
  ```piscript
  println("Hello,", "world!")  // Output: Hello, world!\n
  println(1, 2, 3)             // Output: 1 2 3\n
  ```

---

### print(value1, value2, ...)

Prints values to the standard output without a newline.

- **Parameters:**

  - `value1, value2, ...` – One or more values to print.

- **Returns:**

  - `null`

- **Behavior:**

  - Concatenates all values as strings with spaces and prints them.
  - Does **not** append a newline at the end.

- **Examples:**
  ```piscript
  print("Hello,")
  print("world!")  // Output: Hello,world!
  ```

---

### printf(format, value1, value2, ...)

Prints formatted text to the standard output.

- **Parameters:**

  - `format` _(string)_ – A format string containing placeholders like `{0}`, `{1}`, etc.
  - `value1, value2, ...` – Values to fill into the format string.

- **Returns:**

  - `null`

- **Behavior:**

  - Replaces `{0}`, `{1}`, etc. in the format string with the corresponding values.
  - Useful for building dynamic strings.

- **Examples:**
  ```piscript
  printf("Hello, {0}! You are {1} years old.\n", "Alice", 30)
  // Output: Hello, Alice! You are 30 years old.
  ```

---

### text(x, y, txt, color)

Draws text to the screen at the given position.

- **Parameters:**

  - `x` _(number)_ – The x-coordinate.
  - `y` _(number)_ – The y-coordinate.
  - `txt` _(string)_ – The text to draw.
  - `color` _(number)_ – The color index (from the 32-color palette).

- **Returns:**

  - `null`

- **Examples:**
  ```piscript
  text(10, 20, "Score: 100", 15)
  ```

---

### `key(keyname, once)`

Checks if a specific key is pressed.

- **Parameters:**

  - `keyname` _(string)_ – The name of the key (e.g., `"left"`, `"a"`, `"space"`).
  - `once` _(boolean, optional)_ – If `true`, returns `true` only the first time the key is pressed.

- **Returns:**

  - `true` if the key is pressed; otherwise `false`.

- **Examples:**

  ```piscript
  if (key("space")) {
    println("Spacebar is held")
  }

  if (key("up", true)) {
    println("Up key was just pressed")
  }
  ```

---

### input(prompt)

Reads input from the user via standard input.

- **Parameters:**

  - `prompt` _(string)_ – A string to display as a prompt to the user.

- **Returns:**

  - A string containing the user's input.

- **Examples:**
  ```piscript
  name = input("Enter your name: ")
  println("Hello, ", name)
  ```

---