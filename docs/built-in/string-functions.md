# String Functions

These helpers operate on PiScript strings. Transform helpers return strings;
tests return booleans.

## `char(code)`

Creates a one-character string from a numeric character code.

```piscript
writeln(char(65))
```

## `ord(text)`

Returns the code of the first character in a non-empty string.

```piscript
writeln(ord("A"))
```

## `trim(text)`

Returns a copy with leading and trailing whitespace removed.

```piscript
let clean = trim("  PiScript  ")
writeln(clean)
```

## `upper(text)`

Returns an uppercase copy of the input string.

```piscript
writeln(upper("pixel"))
```

## `lower(text)`

Returns a lowercase copy of the input string.

```piscript
writeln(lower("PIXEL"))
```

## `replace(text, old, new)`

Replaces occurrences of `old` with `new` and returns the result. The old string
must not be empty.

```piscript
writeln(replace("red red", "red", "blue"))
```

## `split(text, separator)`

Splits text by a delimiter string and returns a list of parts.

```piscript
writeln(split("x,y,z", ","))
```

## `is_upper(text)`

Returns `false` if an alphabetic character is not uppercase. A string with no
lowercase alphabetic characters passes.

```piscript
writeln(is_upper("HP 10"))
```

## `is_lower(text)`

Returns `false` if an alphabetic character is not lowercase.

```piscript
writeln(is_lower("level-1"))
```

## `is_digit(text)`

Returns `true` only when the non-empty string contains digits `0` through `9`
and no other characters.

```piscript
writeln(is_digit("2048"))
writeln(is_digit("20.48"))
```

## `is_numeric(text)`

Checks a signed integer-or-decimal string according to the runtime numeric text
test.

```piscript
writeln(is_numeric("-12.5"))
writeln(is_numeric("12e3"))
```

## `is_alpha(text)`

Returns `true` for a non-empty string containing only alphabetic characters.

```piscript
writeln(is_alpha("PiScript"))
```

## `is_alnum(text)`

Returns `true` for a non-empty string containing only letters and digits.

```piscript
writeln(is_alnum("player2"))
writeln(is_alnum("player_2"))
```
