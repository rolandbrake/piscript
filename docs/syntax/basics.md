# Language Basics

## Statements and Blocks

PiScript accepts one statement per line or statements separated with `;`.
Braces group a multi-statement block. `if`, `while`, and `for` can also take a
single statement body without braces.

```piscript
let score = 0

if key("SPACE", true) {
    score += 1
    writeln("score = " + score)
}
```

## Variables

Use `let` to declare a variable in the current local scope. If no initializer
is given, the variable starts as `nil`.

```piscript
let name = "Ada"
let x = 10, y = 20
let selected
```

PiScript is dynamically typed. A name can hold a number now and another kind of
value later.

```piscript
let score = 12
score = "twelve"
```

## Assignment

`let` and assignment are related but different:

- `let name = value` declares a local variable.
- `name = value` assigns without making a local declaration.
- `name += value`, `name -= value`, `name *= value`, and the other compound
  assignment operators update a name using the matching operator.
- `target <- value` is an inline assignment expression when an assignment must
  live inside a larger expression.

```piscript
let x = 10
x = x + 2
x *= 3

last = 0
let seen = (last <- x)
```

At top level, plain assignment is a natural way to create or update a global
name:

```piscript
title = "menu"
title = title + " screen"
```

Inside a function or a block with local variables, use `let` when the name must
stay local. A variable assigned without `let` in local scope is a global
variable.

```piscript
let lives = 3

fun lose_life() {
    let message = "try again" // local to this function
    game_over = lives <= 1   // global: no let
    lives -= 1               // assignment without let updates the global name
}
```

That rule matters for temporary names. Write `let i = 0` for a local counter;
write `i = 0` only when the program should share `i` through the global scope.

## Scopes

Top-level names are global. `let` declarations inside functions and brace
blocks are local to the current scope. Inner scopes may declare a new local
with the same name as an outer name.

```piscript
let label = "outside"

fun show_label() {
    let label = "inside"
    writeln(label)
}

show_label()
writeln(label)
```

Function parameters are local names too. Closures can read values captured from
their outer scope; plain assignment without `let` still follows PiScript's
global-assignment rule instead of declaring a new local.

## Destructuring

List-style destructuring reads indexed values from the right-hand side.

```piscript
let [left, right] = [7, 9]
[left, right] = [right, left]
```

## Comments

Use C-style comments.

```piscript
// One line

/*
   Several lines
*/
```

## Access and Mutation

Lists and strings use bracket indexing. Maps support bracket lookup and dot
lookup. Negative indexing is supported by sequence lookup.

```piscript
let nums = [3, 5, 8]
let user = {"name": "Mina", "score": 12}

writeln(nums[0])
writeln(nums[-1])
writeln(user.name)
writeln(user["score"])

user.score = user.score + 1
nums[1] = 13
```

## Slices and Ranges

Bracket slices use `start:end` or `start:end:step`. A range expression uses
`start..end` with an optional `:step`.

```piscript
let text = "piscript"
writeln(text[0:2])

for i in 0..10:2 {
    writeln(i)
}
```

The `slice(start, end, step)` and `range(...)` built-ins create the same kinds
of runtime helper values for APIs that need them.

## Screen and Console Text

`print`, `println`, and `printf` draw text on the PiScript screen. `writeln`
writes a line to the host console, which is handy for tests and diagnostics.
Screen text stays in the framebuffer until `draw()` presents it.

```piscript
clear(COLOR_SOFT_BLACK)
println("hello screen", 4, 4, COLOR_SOFT_IVORY)
draw()
```

The screen, frame loop, palette, and drawing APIs are covered in
[Graphics](graphics.md).
