# 🔁 Functional Programming Functions

### map(collection, callback)

Applies a function to each element in a collection and returns a new collection of the results.

- **Parameters:**

  - `collection` _(list or string)_ – The collection to map over.
  - `callback` _(function)_ – A function that takes each element (and optionally index) and returns a new value.

- **Returns:**

  - A new list (or string if input is a string) containing the results of applying the callback to each item.

- **Behavior:**

  - For **lists**: applies the function to each element and returns a new list.
  - For **strings**: applies the function to each character and returns a new string.
  - The original collection is not modified.

- **Examples:**

  ```piscript
  // List example
  nums = [1, 2, 3]
  doubled = map(nums, fn (x) -> x * 2)
  println(doubled)  // [2, 4, 6]

  // String example
  str = "abc"
  upper = map(str, fn (ch) -> to_upper(ch))
  println(upper)  // "ABC"
  ```

---

### filter(collection, callback)

Filters a collection by including only the elements that return `true` from the callback.

- **Parameters:**

  - `collection` _(list or string)_ – The collection to filter.
  - `callback` _(function)_ – A function that returns `true` to keep an element.

- **Returns:**

  - A new list or string containing only the elements that passed the test.

- **Behavior:**

  - For **lists**: returns a new list of values that satisfy the condition.
  - For **strings**: returns a new string made of characters that pass the filter.

- **Examples:**

  ```piscript
  nums = [1, 2, 3, 4]
  even = filter(nums, fn (x) -> x % 2 == 0)
  println(even)  // [2, 4]

  str = "hello"
  vowels = filter(str, fn (ch) -> contains("aeiou", ch))
  println(vowels)  // "eo"
  ```

---

### reduce(collection, callback, initial)

Reduces a collection to a single value by applying a function cumulatively.

- **Parameters:**

  - `collection` _(list or string)_ – The collection to reduce.
  - `callback` _(function)_ – A function `(accumulator, value) -> result`.
  - `initial` _(any)_ – The initial value for the accumulator.

- **Returns:**

  - A single value resulting from the reduction.

- **Behavior:**

  - Applies the callback in a left-to-right fashion.
  - The result of each callback is passed as the accumulator to the next call.

- **Examples:**

  ```piscript
  nums = [1, 2, 3, 4]
  total = reduce(nums, fn (acc, x) -> acc + x, 0)
  println(total)  // 10

  word = reduce(["H", "e", "y"], fn (acc, x) -> acc + x, "")
  println(word)  // "Hey"
  ```

---

### find(collection, callback)

Returns the index of the first element in the collection for which the callback returns `true`.

- **Parameters:**

  - `collection` _(list or string)_ – The collection to search.
  - `callback` _(function)_ – A function `(value, index) => boolean`.

- **Returns:**

  - The index of the first matching element, or `-1` if none found.

- **Behavior:**

  - Iterates through the collection.
  - Applies the callback to each element.
  - Stops and returns the index when the callback returns `true`.

- **Examples:**

  ```piscript
  nums = [3, 7, 9, 10]
  idx = find(nums, fn (x) => x > 8)
  println(idx)  // 2

  str = "openai"
  idx = find(str, fn (ch) => ch == "a")
  println(idx)  // 4
  ```

---