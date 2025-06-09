# π Mathematical Functions

### floor(x)

Rounds a number or list of numbers down to the nearest integer.

- **Parameters:**

  - `x` _(number or list of numbers)_

- **Returns:** Rounded down value(s).
- **Example:**

  ```piscript
  floor(3.7) // 3
  floor([2.9, 4.1]) // [2, 4]
  ```

---

### ceil(x)

Rounds a number or list of numbers up to the nearest integer.

- **Parameters:**

  - `x` _(number or list of numbers)_

- **Returns:** Rounded up value(s).
- **Example:**

  ```piscript
  ceil(3.1) // 4
  ceil([2.2, 4.8]) // [3, 5]
  ```

---

### round(x)

Rounds a number or list of numbers to the nearest integer.

- **Parameters:**

  - `x` _(number or list of numbers)_

- **Returns:** Rounded value(s).
- **Example:**

  ```piscript
  round(2.5) // 3
  round([2.4, 2.6]) // [2, 3]
  ```

---

### rand()

Generates a random floating-point number between `0` and `1`.

- **Returns:** A number in range `[0, 1)`.
- **Example:**

  ```piscript
  r = rand()
  ```

---

### rand_n(size)

Generates a list of random floating-point numbers between `0` and `1`.

- **Parameters:**

  - `size` _(number)_ – Number of random numbers to generate.

- **Returns:** A list of random numbers.
- **Example:**

  ```piscript
  rand_n(3) // [0.32, 0.77, 0.15] (values vary)
  ```

---

### rand_i(min, max)

Generates a random integer between `min` and `max` (inclusive).

- **Parameters:**

  - `min` _(number)_ – Minimum value.
  - `max` _(number)_ – Maximum value.

- **Returns:** A random integer.
- **Example:**

  ```piscript
  rand_i(1, 5) // 3
  ```

---

### sqrt(x)

Calculates the square root.

- **Parameters:**

  - `x` _(number or list of numbers)_

- **Returns:** Square root(s).
- **Example:**

  ```piscript
  sqrt(9) // 3
  sqrt([1, 4, 9]) // [1, 2, 3]
  ```

---

### sin(x) / cos(x) / tan(x)

Trigonometric functions using radians.

- **Parameters:**

  - `x` _(number or list of numbers)_

- **Returns:** Trigonometric result(s).
- **Example:**

  ```piscript
  sin(0) // 0
  cos([0, π]) // [1, -1]
  ```

---

### asin(x) / acos(x) / atan(x)

Inverse trigonometric functions.

- **Parameters:**

  - `x` _(number or list of numbers)_

- **Returns:** Result(s) in radians.
- **Example:**

  ```piscript
  asin(1) // 1.5708 (approx. π/2)
  ```

---

### deg(x) / rad(x)

Converts between radians and degrees.

- **Parameters:**

  - `x` _(number or list of numbers)_

- **Returns:** Converted angle(s).
- **Example:**

  ```piscript
  deg(π) // 180
  rad(180) // π
  ```

---

### sum(list)

Returns the sum of numbers in a list.

- **Parameters:**

  - `list` _(list of numbers)_

- **Returns:** Sum of the elements.
- **Example:**

  ```piscript
  sum([1, 2, 3]) // 6
  ```

---

### exp(x)

Returns `e` raised to the power of `x`.

- **Parameters:**

  - `x` _(number or list of numbers)_

- **Returns:** `e^x` value(s).

---

### log2(x) / log10(x)

Logarithmic functions.

- **Parameters:**

  - `x` _(number or list of numbers)_

- **Returns:** Logarithm base 2 or 10.
- **Example:**

  ```piscript
  log2(8) // 3
  log10(100) // 2
  ```

---

### pow(base, exponent)

Raises a number (or list of numbers) to the specified power.

- **Parameters:**

  - `base` _(number or list)_
  - `exponent` _(number)_

- **Returns:** `base^exponent`
- **Example:**

  ```piscript
  pow(2, 3) // 8
  pow([2, 3], 2) // [4, 9]
  ```

---

### abs(x)

Returns the absolute value.

- **Parameters:**

  - `x` _(number or list)_

- **Returns:** Absolute value(s).

```piscript
  abs(-2) // 2
  abs([-2, 0.5, 4, -1.2]) // [2, 0.5, 4, 1.2]
```

---

### mean(list) / avg(list)

Calculates the average value.

- **Parameters:**

  - `list` _(list of numbers)_

- **Returns:** Arithmetic mean.

```piscript
  avg([10, 20, 30]); // 20
  mean([2, 4, 6, 8]); // 5
```

---

### var(list)

**Description:**  
Calculates the **variance** of a list of numerical values. Variance measures how far each number in the list is from the mean, providing a sense of the data's spread.

- This function returns the **population variance**, not the sample variance.

**Arguments:**

- `list` _(list of numbers)_ – The input list of numeric elements.

**Returns:**

- A **number** representing the variance of the list.

**Formula:**

- variance = sum((x - mean)²) / n

- Where `x` is each element in the list, `mean` is the arithmetic average, and `n` is the total number of elements.

**Example:**

```piscript
var([2, 4, 4, 4, 5, 5, 7, 9]); // 4
```

---

### dev(list) / std(list)

**Returns:**  
The **standard deviation** of the list — a measure of how spread out the numbers are.

**Formula:**  
σ = √[ (1/n) Σ(x_i - x̄)² ]

Where:
- `x̄` is the mean (average) of the list.
- `x_i` are the individual values.
- `n` is the number of elements in the list.

**Example:**

```piscript
dev([2, 4, 4, 4, 5, 5, 7, 9])  // Returns: 2
std([1, 2, 3, 4, 5])           // Returns: 1.4142...
```
---

### median(list)

**Description:**  
Returns the **median** of a list of numbers. The median is the middle value when the list is sorted.  
If the list has an even number of elements, it returns the average of the two middle values.

**Behavior:**
- The input list must contain only numeric values.
- The list is automatically sorted before calculating the median.
- If the list is empty, return `NaN` or raise an error (based on your language design).

**Algorithm:**
1. Sort the list.
2. Find the middle index.
3. If the list length is odd, return the element at the middle index.
4. If the list length is even, return the average of the two middle elements.

**Examples:**
```piscript
median([5, 2, 1, 4, 3])      // Returns: 3
median([1, 2, 3, 4, 5, 6])   // Returns: 3.5
median([10])                 // Returns: 10
median([])                   // Returns: NaN or error

```
---

### mode(list)

**Description:**  
Returns the **mode** of a list — the value or values that occur **most frequently**.

- If one value appears more than others, that value is returned.
- If multiple values share the highest frequency, a list of all those values is returned (in sorted order).
- If the input list is empty, return `NaN` or raise an error (based on your language rules).

**Behavior:**
- The input list can contain duplicates.
- The return type can be either:
  - A single number (if one mode exists), or
  - A list of numbers (if there are multiple modes).

**Examples:**
```piscript
mode([1, 2, 2, 3, 4])          // Returns: 2
mode([1, 1, 2, 2, 3, 3])       // Returns: [1, 2, 3]
mode([7, 7, 7, 2, 2, 3])       // Returns: 7
mode([])                       // Returns: NaN or error
```
---

### max(list) / min(list)

**Description:**  
Returns the **largest** (`max`) or **smallest** (`min`) number in a list.

- `max(list)` → the maximum value
- `min(list)` → the minimum value

**Behavior:**
- The input list must contain at least one numeric value.
- If the list is empty, the function should return `NaN` or raise an error.
- Works on any unsorted numeric list.

**Examples:**
```piscript
max([5, 3, 9, 1])       // Returns: 9
min([5, 3, 9, 1])       // Returns: 1

max([-10, -3, -50])     // Returns: -3
min([-10, -3, -50])     // Returns: -50

max([42])               // Returns: 42
min([42])               // Returns: 42

max([])                 // Returns: NaN or error
min([])                 // Returns: NaN or error

```
---

All of the above statistical functions (`mean`, `var`, etc.) expect a list of numbers and return a number representing the computed value.

---
