This section provides a detailed reference for all built-in functions and modules available in PiScript. Each entry includes the function signature, parameters, return value, and usage examples.

---

## Math Functions

### `sqrt(x)`

Returns the square root of `x`.

```piscript
sqrt(9)  # 3.0
```

### `abs(x)`

Returns the absolute value of `x`.

```piscript
abs(-5)  # 5
```

### `pow(a, b)`

Returns `a` raised to the power of `b`.

```piscript
pow(2, 3)  # 8
```

---

## Matrix Functions

### `pi_zeros(rows, cols)`

Creates a matrix filled with zeros.

```piscript
pi_zeros(2, 3)
# [[0, 0, 0], [0, 0, 0]]
```

### `pi_eye(n)`

Creates an identity matrix of size `n x n`.

```piscript
pi_eye(3)
# [[1, 0, 0], [0, 1, 0], [0, 0, 1]]
```

### `pi_dot(a, b)`

Computes the dot product of two matrices or vectors.

```piscript
pi_dot([1, 2], [3, 4])  # 11
```

---

## Functional Programming

### `map(fn, list)`

Applies function `fn` to each element of `list`.

```piscript
map(x -> x * 2, [1, 2, 3])  # [2, 4, 6]
```

### `filter(fn, list)`

Filters `list`, keeping only elements where `fn` returns true.

```piscript
filter(x -> x > 1, [1, 2, 3])  # [2, 3]
```

### `reduce(fn, list, init)`

Reduces `list` to a single value by combining elements with `fn`.

```piscript
reduce((a, b) -> a + b, [1, 2, 3], 0)  # 6
```

---

## Graphics Functions (128x128 Console)

### `pix(x, y, color)`

Sets the pixel at `(x, y)` to a color index (0–31).

### `cls(color)`

Clears the screen to the given color.

### `rect(x, y, w, h, color)`

Draws a rectangle at `(x, y)` with width `w`, height `h`.

---

This section is a work-in-progress and will be expanded with more function references as PiScript evolves.
