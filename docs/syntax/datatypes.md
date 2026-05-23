# Data Types

PiScript is dynamically typed. A variable name does not carry a declared type;
the value currently stored in it decides which operations are valid.

```piscript
let value = 12
value = "twelve"
value = [value, true]
```

Use `type(value)` when a program needs a runtime type name and the `is_*`
built-ins for focused checks.

## Type Overview

| Type | Literal or creator | Main use |
| --- | --- | --- |
| Number | `42`, `3.14`, `10e-2`, `0xff00ffff` | Arithmetic, time, coordinates, colors, bitwise values |
| Boolean | `true`, `false` | Conditions and flags |
| Nil | `nil` | Absence and default placeholder |
| String | `"hello"`, `'hello'` | Text and character sequences |
| List | `[1, "two", true]` | Ordered mutable collections and vectors |
| Map | `{name: "Pi", score: 3}` | Keyed data and object prototypes |
| Range | `0..10`, `range(10)` | Numeric iteration |
| Slice | `slice(1, 6, 2)` | Reusable sequence slice selector |
| Function | `fun f() {}`, `x -> x + 1` | Callable values and closures |
| File | `open("save.txt")` | File I/O handle |
| Image | `image("tile.png")` | 2D image pixels and alpha |
| Sound | `tone(...)`, `sound(...)` | Audio asset or generated sound |
| Model | `load3d("ship.obj")` | 3D mesh triangles |

## Numbers

PiScript exposes one user-facing numeric type. Integer-looking and
floating-point-looking literals both participate in numeric operations.
Scientific decimal notation is accepted.

```piscript
let lives = 3
let dt = 10e-2
let gravity = 9.81
let rgba = 0x80ff00ff
```

Number literals also support base prefixes. Hexadecimal is especially useful
for packed colors written as `0xAARRGGBB`.

```piscript
let mask = 0b1111
let octal_value = 0o17
let magenta = 0xffff00ff
```

`INF` and `NAN` are language values. Division by zero yields infinity in the
current VM path, while remainder by zero yields `NAN`.

```piscript
writeln(1 / 0)
writeln(4 % 0)
```

Bitwise operators convert numeric operands through integer operations. Keep
that distinction in mind when a number was produced by floating-point math.

## Booleans and Truthiness

Boolean literals are `true` and `false`. Conditions call the runtime truthiness
conversion used by `as_bool`.

| Value | Truthiness |
| --- | --- |
| `false`, `0`, `nil` | false |
| Empty string, list, map, or zero-length range | false |
| Non-zero number | true |
| Non-empty string, list, map, or range | true |
| Other runtime objects such as images or functions | true |

```piscript
if [1, 2, 3]
    writeln("the list has items")

if !nil
    writeln("nil is falsey")
```

## Nil

`nil` represents absence. A `let` declaration with no initializer receives
`nil`, a function parameter without a supplied argument starts from its default
or `nil`, and effect-only built-ins commonly return `nil`.

```piscript
let selected
writeln(selected == nil)

fun maybe_name(name) {
    if name == nil
        return "anonymous"
    return name
}
```

## Strings

Strings may use single or double quotes. They are sequences of characters for
indexing, slicing, iteration, membership tests, concatenation, repetition, and
string built-ins.

```piscript
let title = "PiScript"
writeln(title[0])
writeln(title[-1])
writeln(title[0:2])
writeln("Script" in title)
```

Sequence indexing normalizes negative indices. The VM also wraps sequence
indices through its index helper, so stay within normal bounds when clarity
matters.

Several collection helpers can mutate string contents (`push`, `pop`,
`insert`, `remove`, `append`, and `unshift`), while string transform helpers
such as `upper` and `replace` return new strings.

```piscript
let word = "pi"
push(word, "!")
writeln(word)
writeln(upper(word))
```

## Lists

Lists are mutable ordered collections. Items may use different types.

```piscript
let mixed = [1, "two", true, nil]
mixed[0] = 9
push(mixed, {"name": "last"})
writeln(mixed[-1])
```

Lists are iterable and sliceable.

```piscript
let scores = [5, 8, 13, 21]
for score in scores {
    writeln(score)
}
writeln(scores[1:3])
```

Numeric lists also act as vectors. Nested numeric lists with compatible shape
are used as matrices by matrix functions and by list-to-list `*`
multiplication.

```piscript
let a = [1, 2, 3]
let b = [4, 5, 6]
writeln(a @ b)
writeln(a ^ b)
```

## Maps

Maps store keyed values. Literal keys may be identifiers, strings, numbers, or
booleans; the implementation stores map keys through string-keyed table access.
Identifier members can be read with dot syntax.

```piscript
let player = {
    name: "Mina",
    "hp": 8,
    1: "slot one"
}

writeln(player.name)
writeln(player["hp"])
player.hp += 1
```

Maps are iterable. Iterating a map yields values in the current runtime path.
Use `keys(map)` when you need keys explicitly.

Maps are also PiScript's object substrate. Map methods, `constructor`, `this`,
`clone`, and prototype checks are covered in [Objects](OOP.md).

## Ranges and Slices

A range is an iterable numeric span. It includes the start and stops before the
end. The step may be positive or negative and cannot be zero.

```piscript
for i in 0..6 {
    writeln(i)
}

for i in 6..0:-2 {
    writeln(i)
}
```

`range(end)`, `range(start, end)`, and `range(start, end, step)` build the
same runtime range form.

A slice selects a subsequence. Inline slices are the most common form:

```piscript
let letters = "abcdef"
writeln(letters[1:5])
writeln(letters[0:6:2])
```

`slice(start, end, step)` returns a slice value if a slice must be created and
passed around before use.

## Functions

Functions are first-class objects. Named functions, anonymous `fun`
expressions, and arrow functions all produce callable values.

```piscript
fun add(a, b) {
    return a + b
}

let double = x -> x * 2
let funcs = [add, double]
writeln(funcs[0](3, 4))
```

Function values can capture values from their outer scope. The full call model,
default parameters, `args`, `kw_args`, and named arguments are documented in
[Functions](functions.md).

## Files and Media Objects

Files, images, sounds, and 3D models are opaque runtime objects. They are not
created with literals; specialized built-ins create and consume them.

```piscript
let data = open("save.txt", "r")
let tile = image("assets/tile.png")
let beep = tone(440, 120, WAVE_SINE)
let mesh = load3d("assets/ship.obj")
```

Images can be transformed by the 2D media functions. Sounds are played through
audio functions. Models flow through 3D transform and render functions before
`draw()` presents the screen.

## Checking and Converting Values

```piscript
writeln(type([1, 2, 3]))
writeln(is_list([1, 2, 3]))
writeln(as_num("12.5"))
writeln(as_str(true))
writeln(as_bool(""))
```

`as_num` requires a value the runtime can parse as numeric. `as_str` uses the
runtime string representation, including readable output for lists, maps, nil,
and numeric special values.
