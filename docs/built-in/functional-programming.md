# Functional Programming

Functional helpers take a collection first and a callback second. The callback
is an ordinary PiScript function value, so named functions, anonymous
functions, and arrow functions all work.

## `map(list, fn)`

`map` calls `fn(item)` once for every item in the input list and returns a new
list of callback results. It does not mutate the input list.

```piscript
let nums = [1, 2, 3]
let squares = map(nums, x -> x * x)
writeln(squares)
```

## `filter(list, fn)`

`filter` calls `fn(item)` for each list item. Items whose callback result is
truthy are copied into a new list.

```piscript
let nums = [1, 2, 3, 4, 5]
let odd = filter(nums, x -> x % 2 != 0)
writeln(odd)
```

## `reduce(list, fn[, initial])`

`reduce` folds a list into one value by calling
`fn(accumulator, item)`. If `initial` is omitted, the first list item becomes
the initial accumulator and iteration starts at the second item.

```piscript
let nums = [2, 4, 6]
let total = reduce(nums, (acc, item) -> acc + item, 0)
writeln(total)
```

## `find(collection, fn)`

`find` searches a list or string. It calls the callback with each list item or
one-character string and returns the first matching index. It returns `-1`
when no callback result is truthy.

```piscript
let first_large = find([3, 8, 13], x -> x > 10)
let first_digit = find("pi3", ch -> is_digit(ch))
writeln(first_large)
writeln(first_digit)
```
