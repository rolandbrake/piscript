# Built-in Functions

Every function registered by the VM is documented in the category pages below.
Built-ins are global; call them without importing a module.

| Category | Reference |
| --- | --- |
| Math and statistics | [Mathematical functions](built-in/mathematical-functions.md) |
| Collections and ranges | [Collection functions](built-in/collection-functions.md) |
| Functional helpers | [Functional programming](built-in/functional-programming.md) |
| Strings | [String functions](built-in/string-functions.md) |
| Screen text, input, and files | [I/O functions](built-in/io-functions.md) |
| Pixel graphics and frame control | [Graphics functions](built-in/graphics-functions.md) |
| Images, audio, and 3D | [Media functions](built-in/media-functions.md) |
| Matrices and vectors | [Matrix manipulation](built-in/matrix-manipulation.md) |
| Maps and object helpers | [Map functions](built-in/map-functions.md) |
| Runtime types | [Type functions](built-in/type-functions.md) |
| Clock and delay | [Time functions](built-in/time-functions.md) |
| Runtime and scripts | [System functions](built-in/system-functions.md) |

## Conventions

- `nil` means a built-in performs an effect without a value result.
- Signatures in brackets are optional parts, for example
  `draw([offset_x, offset_y])`.
- `list` means a PiScript list. `collection` means the function accepts the
  collection types listed in its row.
- Screen text functions draw into the screen buffer. `writeln` is the console
  output function.
- Relative paths used by script/media helpers are resolved by the runtime
  according to the active script where the implementation supports it.
