# Type Functions

These functions allow inspection of data types at runtime.

---

### type(value)

**Description:**  
Returns a string representing the **type** of the input `value`.

**Arguments:**
- `value` *(any)* – The value to inspect.

**Returns:**
- *(string)* – A string indicating the type of the input.

**Possible Return Values:**
- `"number"` – For numeric values (integers or floats).
- `"string"` – For text.
- `"bool"` – For `true` or `false`.
- `"list"` – For lists (arrays).
- `"function"` – For callable functions.
- `"null"` – For a null/none value.

**Examples:**
```piscript
print(type(42))          // "number"
print(type("hello"))     // "string"
print(type([1, 2, 3]))    // "list"
print(type(true))        // "bool"
print(type(null))        // "null"
print(type(type))        // "function"
```
---


### Type Checking Functions

These functions check whether a given value is of a specific type and return `true` or `false`.

| Function    | Checks if the value is a...         |
|-------------|-------------------------------------|
| `is_list`   | List (array)                        |
| `is_map`    | Map (key-value dictionary)          |
| `is_num`    | Number (integer or float)           |
| `is_str`    | String                              |
| `is_bool`   | Boolean (`true` or `false`)         |

---

**Arguments:**  
- `value` *(any)* – The value to check.

**Returns:**  
- *(bool)* – `true` if `value` matches the type, otherwise `false`.

---

**Examples:**

```piscript
print(is_list([1, 2, 3]))        // true
print(is_map({ "a": 1, "b": 2 })) // true
print(is_num(42))                 // true
print(is_str("hello"))            // true
print(is_bool(false))             // true

print(is_list("not a list"))     // false
print(is_map([1, 2, 3]))          // false
print(is_num("42"))               // false
print(is_str(123))                // false
print(is_bool("true"))            // false
```
---


### Type Conversion Functions

These functions attempt to **convert** a given value to a specific type. If conversion is not possible, the behavior depends on the implementation (may return `null`, throw an error, or convert to a default).

| Function   | Converts value to...         |
|------------|------------------------------|
| `as_num`   | Number (integer or float)    |
| `as_str`   | String                       | 
| `as_bool`  | Boolean (`true` or `false`)  |

---

**Arguments:**  
- `value` *(any)* – The value to convert.

**Returns:**  
- The value converted to the target type if possible, otherwise behavior may vary.

---

**Examples:**

```piscript
print(as_list("abc"))       // ['a', 'b', 'c'] or equivalent
print(as_num("123"))        // 123
print(as_str(456))          // "456"
print(as_bool(0))           // false
print(as_bool("true"))      // true

// Examples of failed conversions may return null or throw error depending on implementation
print(as_num("abc"))        // null or error
```
---