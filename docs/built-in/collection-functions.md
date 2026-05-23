# Collection Functions

Collections include lists, strings, and maps depending on the function.
Mutation behavior matters: list and some string helpers change their first
argument, while helpers such as `copy`, `reverse`, `slice`, and `range` return
separate values.

## End Operations

### `push(collection, value...)`

Appends one or more values to a list and returns the new length. When the target
is a string, appended values must be one-character strings.

```piscript
let nums = [1, 2]
writeln(push(nums, 3, 4))
writeln(nums)
```

### `append(collection, value...)`

Appends one or more values and returns the new length. For strings it can append
string fragments rather than requiring single characters.

```piscript
let label = "hp"
writeln(append(label, ": ", "8"))
writeln(label)
```

### `pop(collection)`

Removes the last list item or last string character and returns it. Empty input
raises a runtime error.

```piscript
let stack = [10, 20, 30]
writeln(pop(stack))
writeln(stack)
```

### `peek(collection)`

Returns the last list item or last string character without removing it.

```piscript
writeln(peek(["first", "last"]))
writeln(peek("abc"))
```

## Front and Indexed Mutation

### `unshift(collection, value...)`

Prepends values and returns the new size. It mutates the target list or string.

```piscript
let path = ["end"]
unshift(path, "start")
writeln(path)
```

### `insert(collection, index, value)`

Inserts a value into a list or text into a string at `index`. List insertion
requires an index between the start and end insertion positions.

```piscript
let nums = [1, 3]
insert(nums, 1, 2)
writeln(nums)
```

### `remove(collection, index)`

Removes and returns one list item or one string character at the index.

```piscript
let word = "pixel"
writeln(remove(word, 1))
writeln(word)
```

## Inspection

### `empty(collection)`

Returns `true` for an empty list, string, or map.

```piscript
writeln(empty([]))
writeln(empty({x: 1}))
```

### `len(collection)`

Returns size for lists, strings, and maps. Unsupported values return `nil` in
the current implementation.

```piscript
writeln(len("screen"))
writeln(len({x: 1, y: 2}))
```

### `contains(collection, value)`

Tests list values, string substrings, or map keys. The `in` operator calls this
logic with reversed expression order.

```piscript
writeln(contains([1, 2, 3], 2))
writeln(contains("piscript", "script"))
writeln("hp" in {hp: 8})
```

### `index_of(collection, value)`

Returns the first index of a list value or string substring, or `-1` if no
match exists.

```piscript
writeln(index_of([4, 8, 12], 8))
writeln(index_of("pixel", "x"))
```

## Ordering and Copies

### `sort(list)`

Sorts a list in place and returns `nil`. Values must all be numbers or all be
strings.

```piscript
let nums = [9, 1, 5]
sort(nums)
writeln(nums)
```

### `reverse(collection)`

Returns a reversed list copy or reversed string. The original list is not the
returned value's storage path.

```piscript
let nums = [1, 2, 3]
let backwards = reverse(nums)
writeln(backwards)
writeln(nums)
```

### `shuffle(list)`

Shuffles list items in place and returns the same list value.

```piscript
let cards = [1, 2, 3, 4]
shuffle(cards)
writeln(cards)
```

### `copy(collection)`

Returns a copy of a list or string. List item values are copied shallowly, so
nested object values may still be shared.

```piscript
let original = [1, 2]
let other = copy(original)
push(other, 3)
writeln(original)
writeln(other)
```

## Range and Slice Values

### `slice(start, end[, step])`

Creates a slice value. Inline bracket slice syntax is shorter for direct access,
but a slice value can be kept and reused.

```piscript
let every_other = slice(0, 6, 2)
let letters = "abcdef"
writeln(letters[every_other])
```

### `range(end)`

Creates an iterable range from `0` to just before `end`.

```piscript
for i in range(3)
    writeln(i)
```

### `range(start, end[, step])`

Creates a range with explicit start and optional non-zero step.

```piscript
for i in range(6, 0, -2) {
    writeln(i)
}
```
