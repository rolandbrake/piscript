# Map and Object Functions

These helpers operate on maps. A PiScript object prototype or object instance
is also represented by a map.

## `clone(map)`

Creates a shallow copy of a map and links the copy's prototype to the original
map. Own keys are copied; nested values are still shared references where the
copied value itself is an object.

```piscript
let base = {x: 1, y: 2}
let moved = clone(base)
moved.x = 9
writeln(moved.x)
writeln(moved is base)
```

## `keys(map)`

Returns a list containing the map's own key strings. Use it when iterating keys
instead of values.

```piscript
let stats = {hp: 8, mp: 3}
writeln(keys(stats))
```

## `values(map)`

Returns a list containing the map's own values.

```piscript
let stats = {hp: 8, mp: 3}
writeln(values(stats))
```

See [Objects](../syntax/OOP.md) for constructors, map methods, `this`, and
prototype-style object code.
