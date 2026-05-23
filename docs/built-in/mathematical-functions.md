# Mathematical Functions

Math functions work with PiScript numbers. Many one-argument transforms also
accept a numeric list and return a list with the same transform applied to each
item.

## Rounding

### `floor(x)`

Returns the greatest whole numeric value not greater than `x`.

```piscript
writeln(floor(3.9))
writeln(floor([1.2, 2.8]))
```

### `ceil(x)`

Returns the smallest whole numeric value not less than `x`.

```piscript
writeln(ceil(3.1))
writeln(ceil([1.2, 2.8]))
```

### `round(x)`

Rounds a number or each numeric list item to the nearest whole value.

```piscript
writeln(round(2.6))
writeln(round([1.1, 1.9]))
```

## Roots, Powers, and Absolute Values

### `sqrt(x)`

Computes square root. List input is transformed item by item.

```piscript
writeln(sqrt(81))
writeln(sqrt([4, 9, 16]))
```

### `pow(base, exponent)`

Raises `base` to `exponent`. Use `**` when operator syntax reads better.

```piscript
writeln(pow(2, 10))
writeln(2 ** 10)
```

### `exp(x)`

Computes `E` raised to `x`.

```piscript
writeln(exp(1))
writeln(exp([0, 1]))
```

### `abs(x)`

Returns absolute magnitude for a number or every item in a numeric list.

```piscript
writeln(abs(-12))
writeln(abs([-3, 0, 3]))
```

## Trigonometry

### `sin(x)`

Computes sine for radians.

```piscript
writeln(sin(PI / 2))
```

### `cos(x)`

Computes cosine for radians.

```piscript
writeln(cos(0))
```

### `tan(x)`

Computes tangent for radians.

```piscript
writeln(tan(rad(45)))
```

### `asin(x)`

Computes arc sine and returns radians.

```piscript
writeln(asin(1))
```

### `acos(x)`

Computes arc cosine and returns radians.

```piscript
writeln(acos(0))
```

### `atan(x)`

Computes arc tangent and returns radians.

```piscript
writeln(atan(1))
```

### `deg(x)`

Converts radians to degrees. This pairs naturally with inverse trig functions.

```piscript
writeln(deg(PI))
```

### `rad(x)`

Converts degrees to radians before calling trig functions.

```piscript
writeln(rad(180))
```

## Logarithms

### `log2(x)`

Computes the base-2 logarithm.

```piscript
writeln(log2(8))
```

### `log10(x)`

Computes the base-10 logarithm.

```piscript
writeln(log10(1000))
```

### `logE(x)`

Computes the natural logarithm.

```piscript
writeln(logE(E))
```

## Statistics

Statistics helpers expect numeric lists.

### `sum(list)`

Adds all list numbers.

```piscript
writeln(sum([3, 5, 8]))
```

### `mean(list)`

Returns the arithmetic mean.

```piscript
writeln(mean([2, 4, 6]))
```

### `avg(list)`

Average helper with the same role as `mean`.

```piscript
writeln(avg([10, 20, 30]))
```

### `var(list)`

Returns numeric variance for the list.

```piscript
writeln(var([2, 4, 6, 8]))
```

### `dev(list)`

Returns standard deviation for the list.

```piscript
writeln(dev([2, 4, 6, 8]))
```

### `median(list)`

Returns the middle statistical value after ordering input values.

```piscript
writeln(median([9, 1, 5]))
```

### `mode(list)`

Returns the most frequent numeric value.

```piscript
writeln(mode([2, 1, 2, 3]))
```

### `min(list)`

Returns the smallest numeric item.

```piscript
writeln(min([7, 3, 9]))
```

### `max(list)`

Returns the largest numeric item.

```piscript
writeln(max([7, 3, 9]))
```

## Random Numbers

### `seed(number)`

Sets the random generator seed. Use a fixed seed when a procedural sequence
must repeat.

```piscript
seed(7)
writeln(rand())
```

### `rand()`

Returns one random number in the range from zero through one.

```piscript
let chance = rand()
writeln(chance)
```

### `rand_n(size)`

Returns a list of random numbers.

```piscript
let noise = rand_n(5)
writeln(noise)
```
