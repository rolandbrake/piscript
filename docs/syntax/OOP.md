# Piscript: Object-Oriented Programming (OOP)

Piscript supports basic object-oriented programming through the use of **maps as objects**. Maps can contain functions, act as constructors, and simulate class-like behavior using the `this` keyword.

---

## 🧱 Defining Objects Using Maps

Objects are defined using map literals with methods and a `constructor` function.

```piscript
let Circle = {
    constructor(x, y, r) {
        this.x = x
        this.y = y
        this.r = r
    },

    area() {
        return 3.14 * this.r * this.r
    }
}

let c = Circle(10, 20, 5)
println(c.area())  # 78.5
```

---

## 🏗️ Constructor Function

The `constructor` method is automatically called when the object is invoked like a function:

```piscript
let c = Circle(1, 2, 3)
```

This is syntactic sugar for:

```piscript
let c = clone(Circle)
c.constructor(1, 2, 3)
```

> `clone(map)` creates a shallow copy and sets up `this` to refer to the new object instance.

---

## 🔁 Methods and `this`

* Inside any method, `this` refers to the object instance.
* Methods can access and mutate internal state through `this`.

```piscript
let Dog = {
  constructor(name) {
    this.name = name
  },
  speak() {
    return this.name + " says woof!"
  }
}

let d = Dog("Fido")
println(d.speak())  // "Fido says woof!"
```

---

## 🧬 Inheritance (Prototype Chaining)

You can simulate inheritance by cloning a parent object and extending it:

```piscript
let Animal = {
  constructor(name) {
    this.name = name
  },
  speak() {
    return this.name + " makes a sound"
  }
}

let Cat = clone(Animal)
Cat.speak = fun() {
  return this.name + " meows"
}

let kitty = Cat("Luna")
println(kitty.speak())  // "Luna meows"
```

---

## 🔍 Inspecting Objects

* You can access fields and methods using dot (`.`) or string key (`[]`) syntax.
* Methods are stored just like fields and can be replaced or redefined at runtime.

```piscript
println(kitty["speak"]())
kitty.color = "black"
println(kitty.color)
```

---

## 🧪 `is` Operator (Instanceof Equivalent)

Piscript supports an `is` operator that works like JavaScript's `instanceof` or Python's `isinstance`:

```piscript
let p = Circle(1, 2, 3)
println(p is Circle)  // true

let q = clone(Circle)
println(q is Circle)  // true
```

### Notes:

* Checks whether an object was created from (cloned from) the given prototype
* Works with inheritance: if `Cat` is cloned from `Animal`, then `Cat("Luna") is Animal` returns true

---

## ✅ Summary

| Feature          | Supported |
| ---------------- | --------- |
| Constructor      | ✅         |
| Methods          | ✅         |
| `this` keyword   | ✅         |
| Prototypes/Clone | ✅         |
| Dynamic fields   | ✅         |
| Method override  | ✅         |
| `is` operator    | ✅         |



## 📌 Notes

* Piscript does not have `class`, `super`, or private members yet.
* Prototype inheritance is manual and flexible.

