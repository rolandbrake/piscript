# PiScript API Reference

The runtime API is available without imports. A built-in name can be called
directly from a `.pi` script.

## Language

| Section | Purpose |
| --- | --- |
| [Basics](syntax/basics.md) | Variables, assignment, scopes, blocks, access, slices |
| [Graphics](syntax/graphics.md) | Screen size, palette, framebuffer, event loop, drawing |
| [Data Types](syntax/datatypes.md) | Runtime values and collection forms |
| [Operators](syntax/operators.md) | Operators and assignment forms |
| [Control Flow](syntax/control-flow-loops.md) | Branching and loops |
| [Functions](syntax/functions.md) | Function syntax and call binding |
| [Objects](syntax/OOP.md) | Maps as prototypes and instances |
| [Constants](syntax/constants.md) | Math, screen, audio, and color constants |

## Built-ins

| Section | Functions |
| --- | --- |
| [Math](built-in/mathematical-functions.md) | Numeric transforms, random values, statistics |
| [Collections](built-in/collection-functions.md) | Lists, strings, maps, range and slice helpers |
| [Functional](built-in/functional-programming.md) | `map`, `filter`, `reduce`, `find` |
| [Strings](built-in/string-functions.md) | Character conversion, case, matching, split |
| [I/O](built-in/io-functions.md) | Screen text, console text, input, keys, files |
| [Graphics](built-in/graphics-functions.md) | Pixel drawing, shapes, palette, frame control |
| [Media](built-in/media-functions.md) | Images, audio, and 3D models |
| [Matrices](built-in/matrix-manipulation.md) | Matrices and vector operations |
| [Maps and Objects](built-in/map-functions.md) | Clone and map inspection |
| [Types](built-in/type-functions.md) | Type names, checks, conversions |
| [Time](built-in/time-functions.md) | Clock and sleep |
| [System](built-in/system-functions.md) | Runtime state, mouse, eval, script execution |

## Value Conventions

- `nil` is the absence value.
- Screen colors accept a palette index from `0` to `79`, a named `COLOR_*`
  constant, or a packed `0xAARRGGBB` number where supported.
- Image, sound, file, and 3D model values are opaque runtime objects.
- Functions marked as mutating in the reference update their input collection;
  transform functions return a new value.
