# Piscript: Function Reference

Piscript supports three types of functions: **named functions**, **arrow functions**, and **anonymous functions**. Each function type supports first-class behavior and closures.

---

## 🧭 Named Functions

Defined using the `fun` keyword.

```piscript
fun f(a = 0, b = 1) {
  println(args)  // Built-in variable holding arguments as list
  return a + b
}

f(3, 4)  # 7
```

### Features:

* Default parameter values (`a = 0`, `b = 1`)
* `args` refers to the list of arguments passed
* `return` is optional (last expression is returned implicitly)

---

## ➡️ Arrow Functions

Shorter syntax for functions using `->`.

```piscript
let f1 = x -> x + 1
let f2 = (x, y) -> x + y
let f3 = (x, y) -> {
  return x + y
}

f1(5)    # 6
f2(3, 4) # 7
f3(3, 4) # 7
```

### Features:

* Concise syntax for one-liners or quick inline logic
* Curly braces `{}` required when using `return` or multiple statements
* Implicit return if the body is a single expression

---

## 🌀 Anonymous Functions

Functions defined without names, assigned to variables or passed directly.

```piscript
let f = fun(a, b) {
  return a + b
}

f(3, 4)  # 7
```

### Features:

* Useful for callbacks, passing functions as arguments
* Identical syntax to named functions but without the name
* Support closures and default parameters

---

## 🧪 Common Behavior

| Feature               | Supported |
| --------------------- | --------- |
| Closures              | ✅         |
| Default arguments     | ✅         |
| First-class functions | ✅         |
| Recursion             | ✅         |
| Higher-order funcs    | ✅         |



## 🔍 Example: Passing Functions

```piscript
fun operate(a, b, op) {
  return op(a, b)
}

let add = (x, y) -> x + y
println(operate(3, 4, add))  # 7
```

---

## 📌 Notes

* Functions can be returned from other functions
* You can store functions in lists, maps, or variables
* All function types can capture variables from the outer scope

