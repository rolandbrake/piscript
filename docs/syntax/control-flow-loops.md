
## 🔄 Control Flow & Loops

### ⚠️ Conditional Statements

Piscript supports `if`, `elif`, and `else` for conditional branching:

```piscript
if score > 90
    println("Excellent")
elif score > 60
    println("Good")
else
    println("Try again")
```

### ⟳ While Loop

Repeats a block as long as the condition is `true`:

```piscript
x = 0
while x < 5 {
    println(x)
    x = x + 1
}
```

### ⏪ For Loop (Range Based)

Use `for ... in` with a range:

```piscript
for i in 0..5:1
    println(i)
```

or you can use `range` function to do the same thing:
```piscript
for i in range(0,5)
    println(i)
```

* Step is optional; default is `1`.

### ⏹ Break

Exits the nearest enclosing loop:

```piscript
for i in 0..10 {
    if i == 5
        break
    println(i)
}
```

### ⏭ Continue

Skips the current iteration and continues the loop:

```piscript
for i in 0..5 {
    if i % 2 == 0
        continue
    println(i)  // prints only odd numbers
}
```

---
