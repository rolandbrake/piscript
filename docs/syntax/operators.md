# Operators

PiScript operators work on dynamic values. This page describes the current VM
behavior, including the cases where an operator has collection or matrix
meaning in addition to its numeric meaning.

## Precedence Shape

From wider expression structure toward tighter grouping, the parser handles
logical OR, logical AND, membership, ranges, bitwise operators, shifts,
comparisons, addition/subtraction, dot product, multiplication/division,
power, unary operators, then member access and calls.

Use parentheses when the intended grouping is not visually obvious.

```piscript
let damage = base + bonus * 2
let inside = (0 <= x) && (x < WIDTH)
```

## Arithmetic Operators

### `+` addition, concatenation, and list append

Two numeric operands are added. If either operand is a string, both sides are
converted to strings and concatenated. If the left side is a list, the right
side is appended to that same list and the list is returned.

```piscript
writeln(2 + 3)
writeln("hp: " + 8)

let items = [1, 2]
items + 3
writeln(items)
```

### `-` subtraction and removal

Numbers subtract normally. For a list on the left, the first equal occurrence
of the right value is removed. For a string on the left, occurrences of the
right string are removed from a new result string.

```piscript
writeln(10 - 4)

let items = [1, 2, 2, 3]
items - 2
writeln(items)
writeln("banana" - "na")
```

### `*` multiplication, repetition, and matrix multiplication

Numbers multiply. A list or string on the left and a numeric right operand
repeats the sequence. Two compatible numeric matrix lists use matrix
multiplication.

```piscript
writeln(6 * 7)
writeln("pi" * 3)
writeln([1, 2] * 2)

let transform = [[1, 0], [0, 1]]
let vector = [[4], [9]]
writeln(transform * vector)
```

### `/`, `%`, and `**`

`/` divides numbers. Division by zero currently returns `INF`. `%` computes an
integer remainder after numeric conversion; remainder by zero returns `NAN`.
`**` raises the left number to the right number.

```piscript
writeln(9 / 2)
writeln(9 % 2)
writeln(2 ** 8)
```

## Vector and Matrix Operators

### `@` and `*.`

`@` and `*.` scan as the dot-product operator. Both sides must be equal-length
numeric lists and the result is a number.

```piscript
let a = [1, 2, 3]
let b = [4, 5, 6]
writeln(a @ b)
writeln(a *. b)
```

### `^`

For two numeric 3D lists, `^` returns their vector cross product. In numeric or
list-bitwise forms it is the XOR operator described below.

```piscript
writeln([1, 0, 0] ^ [0, 1, 0])
```

## Comparisons

| Operator | Meaning |
| --- | --- |
| `==` | Equal |
| `!=` | Not equal |
| `<`, `<=` | Less than and less than or equal |
| `>`, `>=` | Greater than and greater than or equal |

PiScript accepts chained comparisons and combines adjacent comparisons with a
logical AND.

```piscript
let score = 7
writeln(0 <= score <= 10)
```

Equality compares value content for numbers, booleans, nil, strings, and lists
according to the value layer. Object categories without deep comparison fall
back to object identity.

## Logical Operators

`&&` and `||` produce boolean results from runtime truthiness. `!` negates
truthiness.

```piscript
let visible = image_ready && !paused
if visible
    draw()
```

Falsy values include `false`, `0`, `nil`, and empty strings, lists, maps, and
ranges. Other non-empty and non-zero values are truthy.

## Membership and Prototype Operators

### `in`

`left in right` dispatches to `contains(right, left)`.

```piscript
writeln(3 in [1, 2, 3])
writeln("scr" in "piscript")
writeln("name" in {name: "Pi"})
```

### `is`

`value is prototype` walks the map prototype chain. It is false for non-map
operands.

```piscript
let Base = {constructor() {}}
let instance = Base()
writeln(instance is Base)
```

## Bitwise Operators

| Operator | Numeric behavior |
| --- | --- |
| `&` | Integer bitwise AND |
| `|` | Integer bitwise OR |
| `^` | Integer bitwise XOR when not used by two vector lists |
| `~` | Integer bitwise NOT |
| `<<` | Integer left shift |
| `>>` | Signed integer right shift |
| `>>>` | Unsigned 32-bit right shift |

For several binary bitwise operators, a list on the left applies the numeric
bitwise operation to each list item with the numeric right operand.

```piscript
writeln(6 & 3)
writeln([1, 2, 3] << 1)
writeln(~5)
```

## Unary Operators

| Operator | Behavior |
| --- | --- |
| `+value` | Numeric conversion with positive sign |
| `-value` | Numeric negation |
| `!value` | Boolean negation |
| `~value` | Bitwise numeric negation |
| `#collection` | Size of list, string, or map |
| `typeof value` | Runtime type name |
| `++target`, `target++` | Increment assignable numeric target |
| `--target`, `target--` | Decrement assignable numeric target |

```piscript
let count = 2
writeln(++count)
writeln(#"pixels")
writeln(typeof [1, 2])
```

## Assignment Operators

`=` stores a value. Update assignments combine a binary operation with a store.

| Family | Operators |
| --- | --- |
| Arithmetic | `+=`, `-=`, `*=`, `/=`, `%=` |
| Power and dot | `**=`, `@=` |
| Bitwise | `&=`, `|=`, `^=` |
| Shift | `<<=`, `>>=`, `>>>=` |
| Logical | `&&=`, `||=` |

```piscript
let x = 4
x *= 3
x >>= 1
writeln(x)
```

PiScript also has inline assignment with `<-`. It assigns while yielding the
assigned value inside an expression.

```piscript
if (line <- input("> ")) {
    writeln("got " + line)
}
```

## Access, Calls, Ranges, and Slices

| Syntax | Meaning |
| --- | --- |
| `value[index]` | List, string, or map item access |
| `value[start:end]` | Sequence slice |
| `value[start:end:step]` | Sequence slice with step |
| `map.name` | Member access |
| `callee(args...)` | Function call or callable prototype map construction |
| `start..end` | Range with default step |
| `start..end:step` | Range with explicit step |

```piscript
let values = [10, 20, 30, 40]
writeln(values[-1])
writeln(values[0:4:2])

for i in 0..5 {
    writeln(i)
}
```

Named arguments are part of call syntax:

```piscript
fun greet(name, message) {
    return message + ", " + name
}

writeln(greet(message = "Hello", name = "Ada"))
```
