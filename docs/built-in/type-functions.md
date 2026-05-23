# Type Functions

Type helpers inspect or convert runtime values. They do not add static type
declarations to the language.

## `type(value)`

Returns the runtime type name string for a value.

```piscript
writeln(type(3.14))
writeln(type([1, 2, 3]))
```

## `is_num(value)`

Returns `true` when `value` can be treated as numeric by the value layer.

```piscript
writeln(is_num(12))
writeln(is_num("not a number"))
```

## `is_str(value)`

Checks whether a value is a string.

```piscript
writeln(is_str("pi"))
writeln(is_str(["pi"]))
```

## `is_bool(value)`

Checks whether a value is specifically a boolean value, not only truthy or
falsey.

```piscript
writeln(is_bool(true))
writeln(is_bool(1))
```

## `is_list(value)`

Checks whether a value is a PiScript list.

```piscript
writeln(is_list([1, 2]))
writeln(is_list({x: 1}))
```

## `is_map(value)`

Checks whether a value is a map. PiScript object instances are maps too.

```piscript
writeln(is_map({name: "Pi"}))
writeln(is_map("Pi"))
```

## `as_num(value)`

Converts numeric-compatible input to a number. Numeric strings can be parsed;
unsupported values raise a runtime error.

```piscript
let size = as_num("12.5")
writeln(size + 1)
```

## `as_str(value)`

Returns the runtime string form of any value the value layer can stringify.
This is useful before text concatenation or console logging.

```piscript
writeln(as_str([1, 2, 3]))
writeln(as_str(nil))
```

## `as_bool(value)`

Converts through PiScript truthiness. Zero, `nil`, and empty collections are
falsey.

```piscript
writeln(as_bool(""))
writeln(as_bool([1]))
```

`is_mat(value)` is documented with matrix functions because it checks matrix
shape rather than a separate primitive type.
