# Control Flow

## Conditions

Conditions may be parenthesized, but parentheses are optional.

```piscript
if score >= 10 {
    writeln("win")
} elif score > 0 {
    writeln("playing")
} else {
    writeln("start")
}

if ready
    draw()
```

## While

`while` repeats while its condition is truthy.

```piscript
let x = 0
while x < 4 {
    writeln(x)
    x++
}
```

## For-In

`for` iterates an iterable value. Ranges, lists, and strings are common loop
inputs.

```piscript
for i in 0..8:2 {
    writeln(i)
}

for ch in "pi"
    writeln(ch)
```

Parentheses are accepted around a `for` header:

```piscript
for (i in range(3)) {
    writeln(i)
}
```

## Break and Continue

`break` exits the nearest loop. `continue` jumps to its next iteration.

```piscript
for n in range(10) {
    if n == 5
        break
    if n % 2 == 0
        continue
    writeln(n)
}
```

## Return

`return` exits a function. A bare `return` returns `nil` except object
constructors return their `this` instance.

```piscript
fun clamp_low(x) {
    if x < 0
        return 0
    return x
}
```
