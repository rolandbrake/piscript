# Time Functions

## `time()`

Returns the current wall-clock time in milliseconds since the Unix epoch. It is
useful for measuring elapsed time or building timestamp-based logic.

```piscript
let start = time()
// do work
writeln("elapsed ms = " + (time() - start))
```

## `sleep(milliseconds)`

Delays execution for a number of milliseconds. Negative values are clamped to
zero by the implementation.

```piscript
writeln("wait")
sleep(250)
writeln("done")
```
