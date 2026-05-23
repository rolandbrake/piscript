# Functions

Functions are first-class values. They can be assigned, returned, passed to
functional built-ins, and capture local values from outer functions.

## Named Functions

```piscript
fun greet(name, message = "Hello") {
    return message + ", " + name + "!"
}

writeln(greet("Alice"))
writeln(greet(message = "Hi", name = "Mina"))
```

Parameters may have default expressions. A parameter without an explicit
default starts from `nil` when the call does not bind it.

## Named Arguments

Call arguments written as `name = value` bind by parameter name, so their order
does not have to match the declaration.

```piscript
fun mix(a, b, c) {
    return [a, b, c]
}

writeln(mix(c = 3, a = 1, b = 2))
```

Positional arguments must come before named arguments. A parameter may not be
bound both ways in the same call.

## Call Locals

Each user function receives two additional locals:

| Name | Value |
| --- | --- |
| `args` | List containing positional arguments passed to the call |
| `kw_args` | Map containing named arguments passed to the call |

```piscript
fun inspect(a = 0) {
    writeln(args)
    writeln(kw_args)
    return a
}

inspect(4)
inspect(a = 5)
```

## Anonymous and Arrow Functions

```piscript
let add = fun(a, b) {
    return a + b
}

let square = x -> x * x
let move = (x, dx = 1) -> x + dx
let clamp = x -> {
    if x < 0
        return 0
    return x
}
```

An arrow expression without braces returns its expression. A braced function
body uses `return` for a value.

## Closures

Nested functions can read and update captured locals according to normal
PiScript variable storage rules.

```piscript
fun make_adder(offset) {
    return x -> x + offset
}

let add_two = make_adder(2)
writeln(add_two(8))
```
