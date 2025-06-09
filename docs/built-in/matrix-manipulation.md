# 🧮 Matrix Manipulation Functions

### zeros(rows, cols)

Creates a matrix filled with zeros.

- **Parameters:**

  - `rows` _(number)_ – Number of rows.
  - `cols` _(number)_ – Number of columns.

- **Returns:** A matrix (list of lists) filled with `0`.
- **Example:**

  ```piscript
  m = zeros(2, 3)
  // [[0, 0, 0], [0, 0, 0]]
  ```

---

### ones(rows, cols)

Creates a matrix filled with ones.

- **Parameters:**

  - `rows` _(number)_ – Number of rows.
  - `cols` _(number)_ – Number of columns.

- **Returns:** A matrix (list of lists) filled with `1`.
- **Example:**

  ```piscript
  m = ones(2, 2)
  // [[1, 1], [1, 1]]
  ```

---

### eye(size)

Creates an identity matrix.

- **Parameters:**

  - `size` _(number)_ – Size of the square identity matrix.

- **Returns:** A `size x size` matrix with `1` on the diagonal and `0` elsewhere.
- **Example:**

  ```piscript
  m = eye(3)
  // [[1, 0, 0], [0, 1, 0], [0, 0, 1]]
  ```

---

### mult(mat, scalar)

Multiplies a matrix by a scalar.

- **Parameters:**

  - `mat` _(matrix)_ – Matrix to scale.
  - `scalar` _(number)_ – Scalar multiplier.

- **Returns:** A new matrix with each element multiplied by the scalar.
- **Example:**

  ```piscript
  mult([[1, 2], [3, 4]], 2)
  // [[2, 4], [6, 8]]
  ```

---

### dot(a, b)

Computes the dot product of two vectors or matrices.

- **Parameters:**

  - `a` _(list or matrix)_ – First operand.
  - `b` _(list or matrix)_ – Second operand.

- **Returns:** A number (for vectors) or a new matrix (for matrices).
- **Example:**

  ```piscript
  dot([1, 2, 3], [4, 5, 6])
  // 32
  ```

---

### cross(a, b)

Computes the cross product of two 3D vectors.

- **Parameters:**

  - `a` _(list of 3 numbers)_ – First vector.
  - `b` _(list of 3 numbers)_ – Second vector.

- **Returns:** A 3-element list representing the cross product.
- **Example:**

  ```piscript
  cross([1, 0, 0], [0, 1, 0])
  // [0, 0, 1]
  ```

---

### rand_m(rows, cols)

Creates a matrix with random values between `0` and `1`.

- **Parameters:**

  - `rows` _(number)_ – Number of rows.
  - `cols` _(number)_ – Number of columns.

- **Returns:** A matrix of random floating-point numbers.
- **Example:**

  ```piscript
  m = rand_m(2, 2)
  // [[0.56, 0.21], [0.78, 0.98]] (values will vary)
  ```

---

### is_mat(value)

Checks if the given value is a matrix.

- **Parameters:**

  - `value` – Any value.

- **Returns:** `true` if the value is a matrix (list of lists), else `false`.
- **Example:**

  ```piscript
  is_mat([[1, 2], [3, 4]])  // true
  is_mat([1, 2, 3])         // false
  ```

---

### size(mat)

Returns the dimensions of a matrix.

- **Parameters:**

  - `mat` _(matrix)_ – Matrix to check.

- **Returns:** A 2-element list `[rows, cols]`.
- **Example:**

  ```piscript
  size([[1, 2, 3], [4, 5, 6]])
  // [2, 3]
  ```

---