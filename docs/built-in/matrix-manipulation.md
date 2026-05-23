# Matrix and Vector Functions

PiScript uses numeric lists and nested numeric lists for vectors and matrices.
Matrix functions expect shapes compatible with the requested operation.

## `zeros(rows, cols)`

Creates a matrix with `rows` rows and `cols` columns filled with zero.

```piscript
writeln(zeros(2, 3))
```

## `ones(rows, cols)`

Creates a matrix filled with one.

```piscript
let mask = ones(2, 2)
writeln(mask)
```

## `eye(rows, cols)`

Creates an identity-style matrix: diagonal cells are `1` and other cells are
`0`.

```piscript
writeln(eye(3, 3))
```

## `size(matrix)`

Returns matrix dimensions as `[rows, cols]`. The argument must already be a
matrix value with runtime matrix metadata.

```piscript
let m = zeros(4, 2)
writeln(size(m))
```

## `mult(a, b)`

Multiplies two matrices. The column count of `a` must match the row count of
`b`.

```piscript
let a = [[1, 2], [3, 4]]
let b = [[5], [6]]
writeln(mult(a, b))
```

The `*` operator uses matrix multiplication for compatible matrix lists too.

## `dot(a, b)`

Returns the dot product of two equal-length numeric vectors.

```piscript
writeln(dot([1, 2, 3], [4, 5, 6]))
```

The `@` and `*.` operators offer dot-product syntax inside expressions.

## `cross(a, b)`

Returns the cross product of two numeric 3D vectors.

```piscript
writeln(cross([1, 0, 0], [0, 1, 0]))
```

Two 3D vector lists may also use the `^` operator.

## `is_mat(value)`

Checks whether a list has matrix-like nested list shape according to the matrix
helper implementation.

```piscript
writeln(is_mat([[1, 2], [3, 4]]))
writeln(is_mat([1, 2, 3]))
```
