
## 🧮 Data Types

Piscript supports several core data types for flexibility and expressive power:

### 🔢 Numbers

Piscript handles integers and floating-point values:

```piscript
x = 42
pi = 3.14
```

### 🔘 Booleans

Piscript uses `true` and `false` for boolean logic:

```piscript
flag = true
if flag
    println("Yes")
```

### 🔤 Strings

Strings can be declared using either double or single quotes:

```piscript
message = "Hello"
alternative = 'World'
```

### 📋 Lists

Lists are ordered collections:

```piscript
nums = [1, 2, 3, 4]
empty = []
```

### 🗺️ Maps (Objects)

Maps store key-value pairs:

```piscript
person = {"name": "Alice", "age": 30}
println(person.name)
```

### 🔁 Ranges

Piscript supports range objects for iteration:

```piscript
for i in 0..10:2
    println(i)
```

* `start..end:step` syntax
* Inclusive start, exclusive end

### 🕳️ Nil

`nil` represents a null or undefined value:

```piscript
x = nil
if x == nil
    println("x is null")
```

### 🧩 Future Types

Planned support for `set` types is under consideration.

---

