# Piscript: Operator Reference

This document describes all supported operations and operators in the Piscript programming language.

---

## 🔢 Mathematical Operations

| Operator | Description               | Example  | Result |
| -------- | ------------------------- | -------- | ------ |
| `+`      | Addition                  | `2 + 3`  | `5`    |
| `-`      | Subtraction               | `5 - 2`  | `3`    |
| `*`      | Multiplication            | `4 * 2`  | `8`    |
| `/`      | Division                  | `10 / 2` | `5.0`  |
| `%`      | Modulus                   | `5 % 2`  | `1`    |
| `**`     | Exponentiation            | `2 ** 3` | `8`    |
| `//`     | Floor division (optional) | `7 // 2` | `3`    |

> **Note:** `1 % 0` returns `NaN` instead of throwing an error, like JavaScript.

---

## 🔁 Shifting Operations

| Operator | Description                              | Example    | Result                |
| -------- | ---------------------------------------- | ---------- | --------------------- |
| `<<`     | Left shift                               | `4 << 1`   | `8`                   |
| `>>`     | Arithmetic right shift (sign-preserving) | `-4 >> 1`  | `-2`                  |
| `>>>`    | Logical right shift (zero-fill)          | `-4 >>> 1` | large positive number |



## 📝 Inline Assignment

| Operator | Description                          | Example           |
| -------- | ------------------------------------ | ----------------- |
| `<-`     | Inline assignment (like Python `:=`) | `if (x <- get())` |



## 🧠 Type and Membership

| Operator | Description                                     | Example        | Result       |
| -------- | ----------------------------------------------- | -------------- | ------------ |
| `in`     | Membership in list, map, or string              | `'a' in 'abc'` | `true`       |
| `is`     | Type or instance check (like Java `instanceof`) | `x is List`    | `true/false` |



## 📏 Length

Use the `len()` function:

```piscript
len([1, 2, 3])      # 3
len("hello")       # 5
len({"a": 1})      # 1
```

---

## 🔐 Logical Operations

| Operator | Description | Example          | Result  |
| -------- | ----------- | ---------------- | ------- |
| `and`    | Logical AND | `true and false` | `false` |
| `or`     | Logical OR  | `true or false`  | `true`  |
| `not`    | Logical NOT | `not true`       | `false` |

> **Note:** These operators use short-circuit evaluation.



## 🔍 Comparison Operations

| Operator | Description           | Example  | Result |
| -------- | --------------------- | -------- | ------ |
| `==`     | Equal                 | `2 == 2` | `true` |
| `!=`     | Not equal             | `2 != 3` | `true` |
| `<`      | Less than             | `2 < 3`  | `true` |
| `<=`     | Less than or equal    | `2 <= 2` | `true` |
| `>`      | Greater than          | `3 > 2`  | `true` |
| `>=`     | Greater than or equal | `3 >= 3` | `true` |



## 🧮 Bitwise Operations

| Operator | Description | Example    | Result |     |     |
| -------- | ----------- | ---------- | ------ | --- | --- |
| `&`      | Bitwise AND | `5 & 3`    | `1`    |     |     |
| \`       | \`          | Bitwise OR | \`5    | 3\` | `7` |
| `^`      | Bitwise XOR | `5 ^ 3`    | `6`    |     |     |
| `~`      | Bitwise NOT | `~5`       | `-6`   |     |     |



## 📚 List and String Operations

| Expression    | Result      | Description                     |
| ------------- | ----------- | ------------------------------- |
| `[1,2,3] + 1` | `[1,2,3,1]` | Append element                  |
| `[1,2,3] - 1` | `[2,3]`     | Remove first occurrence of `1`  |
| `[1,2] * 2`   | `[1,2,1,2]` | Duplicate list                  |
| `'hi' * 3`    | `'hihihi'`  | Repeat string                   |
| `'ab' + 'cd'` | `'abcd'`    | Concatenate strings             |
| `'abc' - 'b'` | `'ac'`      | Remove first occurrence of char |

> **Note:** These behaviors are defined to align with Python/JS hybrid logic.



## 📐 Matrix Operations

| Operator | Description                 | Example  |
| -------- | --------------------------- | -------- |
| `*.`     | Dot product (vector/matrix) | `a *. b` |
| `^`      | Cross product (3D vectors)  | `a ^ b`  |



## 🗺️ Map/Object Member Access

| Syntax       | Description        |
| ------------ | ------------------ |
| `map.key`    | Access field `key` |
| `map["key"]` | Access dynamic key |



## 📌 Future Considerations

* Operator overloading support?
* Type promotion rules?
* Custom iterable and arithmetic behavior?

Let me know if you'd like this expanded into a full section in the Piscript documentation.
