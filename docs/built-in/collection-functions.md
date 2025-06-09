# 📚 Collection Manipulation Functions

### pop(collection)

Removes and returns the last element of a list or the last character of a string.

- **Parameters:**

  - `collection` _(list or string)_ – The collection to operate on. Can be a list or a string.

- **Returns:**

  - If `collection` is a list: the last element is removed from the list and returned.
  - If `collection` is a string: the last character is removed and returned as a new string.

- **Behavior:**

  - The original collection is **mutated** in the case of lists.
  - For strings, since strings are immutable, a new string with the last character removed is returned.

- **Examples:**

  ```piscript
  lst = [1, 2, 3]
  last = pop(lst)     // last = 3, lst = [1, 2]

  str = "hello"
  ch = pop(str)       // ch = "o", str = "hell"
  ```

---

### push(collection, value)

Appends an element to the end of a list or a character to the end of a string.

- **Parameters:**

  - `collection` _(list or string)_ – The collection to modify. Must be a list or a string.
  - `value` _(any or character)_ – The value to append. For strings, `value` should be a single character.

- **Returns:**

  - If `collection` is a list: the modified list with the value appended.
  - If `collection` is a string: a new string with the character appended.

- **Behavior:**

  - Lists are **mutated** directly.
  - Strings are **immutable**, so a new string with the added character is returned.

- **Examples:**

  ```piscript
  lst = [1, 2]
  push(lst, 3)     // lst becomes [1, 2, 3]

  str = "hi"
  str = push(str, "!")  // str becomes "hi!"
  ```

---

### peek(collection)

Returns the last element of a list or the last character of a string without removing it.

- **Parameters:**

  - `collection` _(list or string)_ – The collection to inspect.

- **Returns:**

  - The last element of the list or the last character of the string.
  - If the collection is empty, returns `null` or an equivalent "empty" value.

- **Behavior:**

  - This function is **non-destructive**. It does not modify the original collection.

- **Examples:**

  ```piscript
  lst = [10, 20, 30]
  last = peek(lst)     // last = 30, lst remains [10, 20, 30]

  str = "world"
  ch = peek(str)       // ch = "d", str remains "world"
  ```

---

### empty(collection)

Checks whether a collection (list or string) is empty.

- **Parameters:**

  - `collection` _(list or string)_ – The collection to check.

- **Returns:**

  - `true` if the collection has no elements or characters.
  - `false` otherwise.

- **Behavior:**

  - Works with both mutable (lists) and immutable (strings) collection types.

- **Examples:**

  ```piscript
  empty([])         // true
  empty([1, 2, 3])  // false

  empty("")         // true
  empty("abc")      // false
  ```

---

### sort(list)

Sorts the elements of a list in ascending order.

- **Parameters:**

  - `list` _(list)_ – The list to sort. Elements must be comparable (e.g., numbers or strings).

- **Returns:**

  - The same list, sorted in place in ascending order.

- **Behavior:**

  - The list is **mutated** directly.
  - The sorting is stable: equal elements retain their original relative order.

- **Examples:**

  ```piscript
  nums = [5, 3, 8, 1]
  sort(nums)        // nums becomes [1, 3, 5, 8]

  names = ["zara", "bob", "alice"]
  sort(names)       // names becomes ["alice", "bob", "zara"]
  ```

---

### insert(collection, index, value)

Inserts an element at a specified position in a list, or a character into a specific position in a string.

- **Parameters:**

  - `collection` _(list or string)_ – The collection to modify.
  - `index` _(number)_ – The position at which to insert the value. Must be within `0` and `len(collection)`.
  - `value` _(any or character)_ – The value to insert. For strings, this should be a single character.

- **Returns:**

  - If `collection` is a list: the same list with the element inserted at the given index.
  - If `collection` is a string: a new string with the character inserted at the given index.

- **Behavior:**

  - Lists are **mutated** directly.
  - Strings are **immutable**; a new string is returned with the change.

- **Examples:**

  ```piscript
  // List example
  lst = [10, 20, 40]
  insert(lst, 2, 30)     // lst becomes [10, 20, 30, 40]

  // String example
  str = "helo"
  str = insert(str, 3, "l")  // str becomes "hello"
  ```

---

### remove(collection, index)

Removes and returns the element at a specified index from a list, or the character at that position from a string.

- **Parameters:**

  - `collection` _(list or string)_ – The collection to modify.
  - `index` _(number)_ – The index of the element or character to remove.

- **Returns:**

  - The removed element or character.

- **Behavior:**

  - Lists are **mutated**; the element is removed from the original list.
  - Strings are **immutable**; a new string is returned with the character removed.
  - If the index is out of bounds, the behavior may be undefined or an error may occur, depending on implementation.

- **Examples:**

  ```piscript
  // List example
  lst = [1, 2, 3, 4]
  val = remove(lst, 2)   // val = 3, lst becomes [1, 2, 4]

  // String example
  str = "hello"
  str = remove(str, 1)   // str becomes "hllo"
  ```

---

### unshift(collection, value1, value2, ...)

Prepends one or more values to the beginning of a collection (list or string).

- **Parameters:**

  - `collection` _(list or string)_ – The collection to modify.
  - `value1, value2, ...` _(any)_ – One or more values to insert at the start. For strings, each value must be a single character.

- **Returns:**

  - the new size of the collection [list or string]

- **Examples:**

  ```piscript
  // List example
  lst = [3, 4]
  unshift(lst, 2, 1)     // lst becomes [1, 2, 3, 4]

  // String example
  str = "world"
  unshift(str, "o", "l", "l", "e", "h")   // str becomes "helloworld"
  ```

---

### append(collection, value1, value2, ...)

Appends one or more values to the end of a collection (list or string).

- **Parameters:**

  - `collection` _(list or string)_ – The collection to modify.
  - `value1, value2, ...` _(any)_ – One or more values to append. For strings, each value must be a single character.

- **Returns:**

  - the new size of the collection [list or string]

- **Examples:**

  ```piscript
  // List example
  lst = [1, 2]
  append(lst, 3, 4)     // lst becomes [1, 2, 3, 4]

  println(lst)

  // String example
  str = "hello"
  append(str, " ", "w", "o", "r", "l", "d")   // str becomes "hello world"

  println(str)
  ```

---

### find(collection, callback)

Returns the index of the **first element** in the collection for which the `callback` returns `true`.

- **Parameters:**

  - `collection` _(list or string)_ – The collection to search.
  - `callback` _(function)_ – A function that takes one element (and optionally its index) and returns `true` for a matching element.

- **Returns:**

  - The index _(number)_ of the first matching element, or `-1` if no match is found.

- **Behavior:**

  - Iterates through the collection from start to end.
  - Invokes `callback(value, index)` for each element.
  - Stops and returns the index when the callback returns `true`.
  - Works for both lists and strings.

- **Examples:**

  ```piscript
  // List example
  lst = [5, 12, 18, 25]
  idx = find(lst, fn (x) -> x > 15)    // idx = 2

  // String example (find first vowel)
  str = "hello"
  idx = find(str, fn (ch) -> contains("aeiou", ch))  // idx = 1
  ```

---

### contains(collection, value)

Checks whether a collection contains a given value or key.

- **Parameters:**

  - `collection` _(list, string, or map)_ – The collection to check.
  - `value` _(any)_ – The value to search for (or key, in case of maps).

- **Returns:**

  - `true` if the value is found in the list or string, or as a key in a map.
  - `false` otherwise.

- **Behavior:**

  - For **lists**: returns `true` if the value is present in the list.
  - For **strings**: returns `true` if the value is a substring (not just a character).
  - For **maps**: returns `true` if the value exists as a **key**.

- **Examples:**

  ```piscript
  // List example
  println(contains([1, 2, 3], 2))       // true
  println(contains([1, 2, 3], 4))       // false

  // String example
  println(contains("hello world", "lo"))  // true
  println(contains("hello", "z"))         // false

  // Map example
  m = { "name": "pi", "type": "lang" }
  println(contains(m, "name"))           // true
  println(contains(m, "value"))          // false
  ```

---

### index_of(collection, value)

Returns the index of the first occurrence of a specific value in a list or string.

- **Parameters:**

  - `collection` _(list or string)_ – The collection to search.
  - `value` _(any)_ – The value or character to locate.

- **Returns:**

  - The index _(number)_ of the first matching element, or `-1` if not found.

- **Behavior:**

  - For lists, uses standard equality for comparison.
  - For strings, compares characters directly.
  - Unlike `find`, this function does **not** accept a callback — it checks for exact equality.

- **Examples:**

  ```piscript
  // List example
  lst = [3, 6, 9, 12]
  println(index_of(lst, 9))   // 2

  // String example
  str = "banana"
  println(index_of(str, "a")) // 1
  println(index_of(str, "z")) // -1
  ```

---

### reverse(collection)

Returns a new collection with the elements in reversed order.

- **Parameters:**

  - `collection` _(list or string)_ – The collection to reverse.

- **Returns:**

  - A new list or string with the same elements/characters in reverse order.

- **Behavior:**

  - Does not modify the original collection.
  - For strings, returns a reversed string.
  - For lists, returns a reversed list.

- **Examples:**

  ```piscript
  // List example
  lst = [1, 2, 3]
  rev = reverse(lst)       // rev = [3, 2, 1]

  // String example
  rev_str = reverse("abc") // rev_str = "cba"
  ```

---

### shuffle(list)

Returns a new list with elements shuffled in random order.

- **Parameters:**

  - `list` _(list)_ – The list to shuffle.

- **Returns:**

  - A new list with the same elements in random order.

- **Behavior:**

  - Only works with lists. If a non-list is passed, an error is thrown.
  - The original list is not modified.

- **Examples:**
  ```piscript
  lst = [1, 2, 3, 4]
  shuffled = shuffle(lst)
  println(shuffled)  // e.g., [3, 1, 4, 2]
  ```

---

### copy(collection)

Returns a shallow copy of a list or string.

- **Parameters:**

  - `collection` _(list or string)_ – The collection to copy.

- **Returns:**

  - A new list or string containing the same elements/characters as the original.

- **Behavior:**

  - For **lists**, returns a new list with the same elements (shallow copy).
  - For **strings**, returns a new string with the same characters.
  - The original collection remains unmodified.

- **Examples:**

  ```piscript
  // List example
  a = [1, 2, 3]
  b = copy(a)
  push(b, 4)
  println(a)  // [1, 2, 3]
  println(b)  // [1, 2, 3, 4]

  // String example
  s = "hello"
  t = copy(s)
  println(t)  // "hello"
  ```

---

### slice(data, start, end)

**Description:**  
Returns a subset (slice) of the input `data`, which can be a **string** or a **list**, from the index `start` up to but **not including** `end`.

**Arguments:**
- `data` *(string | list)* – The input string or list to slice.
- `start` *(int)* – The starting index of the slice (inclusive).
- `end` *(int)* – The ending index of the slice (exclusive).

**Returns:**
- *(string | list)* – A new string or list containing elements from `start` to `end - 1`.

**Behavior:**
- Indexing is zero-based.
- If `start` is omitted or negative, it defaults to `0`.
- If `end` is omitted or exceeds the length of `data`, it defaults to the length of `data`.
- Supports negative indices to count from the end (`-1` is the last element).
- Does not modify the original `data`.

**Examples:**
```piscript
// String slicing
print(slice("Hello, world!", 0, 5))     // Output: "Hello"
print(slice("Hello, world!", 7, 12))    // Output: "world"

// List slicing
print(slice([1, 2, 3, 4, 5], 1, 4))     // Output: [2, 3, 4]
print(slice([10, 20, 30, 40], 0, 2))    // Output: [10, 20]

// Using negative indices
print(slice("abcdef", -4, -1))           // Output: "cde"
print(slice([10, 20, 30, 40, 50], -3, -1)) // Output: [30, 40]
```
---

### range([start], end, [step])

Generates a sequence of numbers.

- **Parameters:**

  - `start` _(optional, number)_ – Start of the sequence. Defaults to `0` if not provided.
  - `end` _(number)_ – End of the sequence (exclusive).
  - `step` _(optional, number)_ – Step size. Defaults to `1`.

- **Returns:**
  A list of numbers from `start` to `end - 1` in increments of `step`.

- **Examples:**
  ```piscript
  range(5)        // [0, 1, 2, 3, 4]
  range(2, 6)     // [2, 3, 4, 5]
  range(1, 10, 2) // [1, 3, 5, 7, 9]
  ```

---