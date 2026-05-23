# PiScript Language Guide

PiScript is a small dynamic language for pixel graphics, games, math, and
creative coding. Programs use expressions, brace-delimited blocks, first-class
functions, lists, maps, ranges, and a built-in 128x128 screen runtime.

## Start Here

- [Language Basics](syntax/basics.md) covers variables, blocks, comments,
  assignment, scopes, destructuring, member access, and slices.
- [Graphics](syntax/graphics.md) explains the 128x128 screen, palette,
  framebuffer, event loop, screen text, and drawing API.
- [Data Types](syntax/datatypes.md) explains values such as numbers, strings,
  lists, maps, ranges, images, sounds, and functions.
- [Operators](syntax/operators.md) lists arithmetic, comparison, logical,
  bitwise, assignment, range, slice, membership, and matrix operators.
- [Control Flow](syntax/control-flow-loops.md) covers `if`, `elif`, `else`,
  loops, `break`, `continue`, and `return`.
- [Functions](syntax/functions.md) covers named functions, anonymous
  functions, arrow functions, defaults, closures, `args`, and named arguments.
- [Objects](syntax/OOP.md) explains PiScript's prototype-style OOP model based
  on callable maps, methods, `constructor`, `this`, and `clone`.
- [Constants](syntax/constants.md) lists the built-in math, screen, audio, and
  color constants.

## Small Program

```piscript
fun move(pos, dx = 0, dy = 0) {
    return [pos[0] + dx, pos[1] + dy]
}

let player = move([60, 60], dy = 2, dx = -1)

clear(COLOR_SOFT_BLACK)
circ(player[0], player[1], 4, COLOR_NEON_GREEN, true)
draw()
```

## Reference

The [built-in function reference](built-in%20functions.md) is organized by
runtime area and documents every built-in registered by the VM.
