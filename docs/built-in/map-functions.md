## 🗺️ Map Utility Functions

### 🔑 keys(map)

**Description:**
Returns a **list of keys** from the given map (object).

**Parameters:**

* `map`: A map (object) to extract the keys from.

**Returns:**

* A list of strings representing the keys in the map.

**Example:**

```piscript
let obj = {
  name: "PiScript",
  year: 2025
}

println(keys(obj))  // Output: ["name", "year"]
```

---

### 📦 values(map)

**Description:**
Returns a **list of values** from the given map (object).

**Parameters:**

* `map`: A map (object) to extract the values from.

**Returns:**

* A list of values in the order of their corresponding keys.

**Example:**

```piscript
let obj = {
  name: "PiScript",
  year: 2025
}

_keys = keys(obj)  // Output: ["name", "year"]

_keys[0] = 'age'

println(_keys)

println(obj)
```


### clone(map)

**Description:**  
Returns a **shallow copy** of the given map (object). The clone will have the same keys and values, and maintain the prototype (`proto`) link of the original, enabling inheritance-like behavior.

**Parameters:**  
- `map` *(Map)*: The map object to clone.

**Returns:**  
- *(Map)*: A new map object containing the same key-value pairs as the original.

**Example:**
```pi
let original = {
  name: "Alice",
  age: 25
}

let copy = clone(original)

println(copy.name)  // Output: Alice
println(copy.age)   // Output: 25

// Changes to the copy won't affect the original
copy.name = "Bob"
println(original.name)  // Output: Alice
println(copy.name)      // Output: Bob
```