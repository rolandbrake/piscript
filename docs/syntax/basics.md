
## 📝 Basics

### 🧾 Variable Declaration

Piscript supports two kinds of variables:

#### 🔒 Local Variables

Use the `let` keyword to declare a **local** variable inside a block or function:

```piscript
fun greet(name) {
    let message = "Hello, " + name
    println(message)
}
```

* Variables declared with `let` are **block-scoped**.
* Re-declaring a `let` variable in the same scope causes an error.

#### 🌍 Global Variables

Variables defined **without `let`** are considered **global**:

```piscript
count = 0  // This is global

fun increment() {
    count = count + 1  // Modifies global count
}
```

> ⚠️ If a variable is assigned without `let`, Piscript assumes you are assigning to a global—even inside a function.

#### 💡 Examples

```piscript
let name = "Alice"       // Local variable
score = 42               // Global variable

fun example() {
    let local = 10
    total = score + local  // total becomes global
}
```

---

### 💬 Comments

Piscript supports two types of comments:

#### 🟩 Single-Line Comments

Use `//` at the beginning of a line:

```piscript
// This is a comment
println("Hello")  // Inline comment
```

#### 🟨 Multi-Line Comments

Use `\*` to start and `*\` to end:

```piscript
\*
This is a multi-line comment.
Useful for documentation or disabling blocks of code.
*\
```

---

### ✅ Quick Recap

| Feature             | Syntax Example         | Notes                    |
| ------------------- | ---------------------- | ------------------------ |
| Local variable      | `let x = 10`           | Scoped to function/block |
| Global variable     | `x = 10`               | Always global            |
| Single-line comment | `// This is a comment` | Starts with `//`         |
| Multi-line comment  | `\* ... *\`            | Encloses multiple lines  |

---

