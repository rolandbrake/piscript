# Getting Started

PiScript is a compact scripting language and SDL runtime for 128x128 pixel
programs. It is useful for small games, creative coding, math sketches, and
teaching language/runtime ideas in a small codebase.

## What You Get

- A dynamic language with variables, lists, maps, ranges, functions, closures,
  named arguments, and prototype-style objects.
- Pixel graphics, screen text, images, audio, keyboard and mouse input.
- Math, statistics, collection, functional, matrix, file, and system built-ins.
- Native desktop builds and an Emscripten web build.

## Build

Native builds need GCC or another C compiler plus SDL2, SDL2_image, and
SDL2_mixer. The Makefile exposes the normal targets:

```bash
make release
make debug
make emscripten
```

On Windows with MinGW the make command may be named `mingw32-make`.

## Run a Script

Run the built interpreter with a PiScript file:

```bash
piscript test.pi
```

The native shell shows the screen window after a script completes. A game
usually keeps drawing frames from a loop.

## First Script

```piscript
let position = [64, 64]

clear(COLOR_SOFT_BLACK)
circ(position[0], position[1], 8, COLOR_SKY_BLUE, true)
println("hello pi", 4, 4, COLOR_SOFT_IVORY)
draw()
```

## Where to Go Next

- Read the [Language Guide](syntax.md).
- Browse the [API Reference](api-reference.md).
- Use the [Built-in Function Reference](built-in%20functions.md) when working
  with graphics, audio, files, collections, matrices, or runtime helpers.
