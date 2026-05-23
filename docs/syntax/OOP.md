# Objects

PiScript does not require class syntax for object-oriented code. Its object
model is based on maps, map methods, prototype cloning, callable prototypes,
and the method-local `this` value.

## Maps with Methods

Inside a map literal a method uses `name(params) { ... }` syntax.

```piscript
let Counter = {
    constructor(start = 0) {
        this.value = start
    },

    inc(amount = 1) {
        this.value += amount
        return this.value
    }
}
```

Calling a prototype map constructs an instance and calls `constructor` when it
exists.

```piscript
let score = Counter(10)
writeln(score.inc())
writeln(score.value)
```

Constructor return values are the instance itself. Use `this` to initialize
and mutate instance fields.

## Access and Replacement

Members use dot or bracket syntax. Methods are values stored on the map, so
they can be replaced like other members.

```piscript
score.label = "round one"
writeln(score["label"])
```

## Clone and Prototypes

`clone(map)` creates a shallow map copy whose prototype points at the original
map. This is the base for manual inheritance and object variation.

```piscript
let FastCounter = clone(Counter)
let fast = FastCounter(4)
writeln(fast.inc())
writeln(fast is Counter)
```

## Object Helpers

| Helper | Purpose |
| --- | --- |
| `clone(map)` | Create a shallow prototype-linked clone |
| `keys(map)` | Return own keys |
| `values(map)` | Return own values |
| `is` | Test prototype/object relation in an expression |

Maps can also be used as plain dictionaries. Only maps with callable behavior
and methods need to be treated as objects.
