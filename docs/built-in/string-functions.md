# 🔤 String Functions

### char(code)

**Description:**  
Returns the **character** corresponding to the ASCII code `code`.

**Arguments:**
- `code` *(int)* – An integer representing the ASCII code of the desired character (typically 0–255).

**Returns:**
- *(string)* – A single-character string matching the ASCII code provided.

**Behavior:**
- Converts an integer ASCII code to its character representation.
- If `code` is outside the valid ASCII range, behavior depends on implementation (may wrap or return an empty string).

**Examples:**
```piscript
print(char(65))    // Output: "A"
print(char(97))    // Output: "a"
print(char(48))    // Output: "0"
```

---


### ord(char)

**Description:**  
Returns the **ASCII code** (integer) of the given single character `char`.

**Arguments:**
- `char` *(string)* – A single-character string whose ASCII code is to be returned.

**Returns:**
- *(int)* – The ASCII code of the character.

**Behavior:**
- Converts a single character into its corresponding ASCII numeric value.
- If the input string is empty or longer than one character, behavior depends on implementation (may return an error or only the first character’s code).

**Examples:**
```piscript
print(ord("A"))    // Output: 65
print(ord("a"))    // Output: 97
print(ord("0"))    // Output: 48
```
---

### trim(string)

**Description:**  
Removes all leading and trailing whitespace characters from the given `string`.

**Arguments:**
- `string` *(string)* – The input string to be trimmed.

**Returns:**
- *(string)* – A new string with leading and trailing whitespace removed.

**Behavior:**
- Whitespace characters typically include spaces, tabs, and newline characters.
- Internal whitespace (between words) remains unchanged.
- Useful for cleaning user input or processing text data.

**Examples:**
```piscript
print(trim("   hello world   "))    // Output: "hello world"
print(trim("no_whitespace"))        // Output: "no_whitespace"
```
---



### upper(string)

**Description:**  
Converts all alphabetic characters in the given `string` to their **uppercase** equivalents.

**Arguments:**
- `string` *(string)* – The input string to convert.

**Returns:**
- *(string)* – A new string where all lowercase letters have been converted to uppercase.

**Behavior:**
- Only affects alphabetic characters `a` to `z`.
- Non-alphabetic characters (numbers, punctuation, whitespace) remain unchanged.
- Useful for case-insensitive comparisons or formatting text.

**Examples:**
```piscript
print(upper("hello world"))    // Output: "HELLO WORLD"
print(upper("PiScript123!"))   // Output: "PISCRIPT123!"
print(upper("Already UPPER"))  // Output: "ALREADY UPPER"
```

---

### replace(string, old, new)

**Description:**  
Returns a new string where **all occurrences** of the substring `old` in `string` are replaced with the substring `new`.

**Arguments:**
- `string` *(string)* – The original string.
- `old` *(string)* – The substring to be replaced.
- `new` *(string)* – The substring to replace with.

**Returns:**
- *(string)* – A new string with all occurrences of `old` replaced by `new`.

**Behavior:**
- Replaces every occurrence of `old` (including overlapping ones depending on implementation).
- If `old` is an empty string, behavior may vary (often returns the original string).
- Useful for text processing, formatting, or cleaning data.

**Examples:**
```piscript
print(replace("hello world", "world", "PiScript"))     // Output: "hello PiScript"
print(replace("aaaaa", "aa", "b"))                      // Output: "bba" (depending on overlapping rules)
print(replace("no match here", "xyz", "123"))          // Output: "no match here"
```
---

### is_upper(string)

**Description:**  
Checks if **all alphabetic characters** in the given `string` are uppercase.

**Arguments:**
- `string` *(string)* – The string to check.

**Returns:**
- *(bool)* – `true` if every alphabetic character is uppercase or if the string contains no alphabetic characters; otherwise, `false`.

**Behavior:**
- Non-alphabetic characters (digits, punctuation, whitespace) are ignored.
- An empty string or a string without alphabetic characters returns `true`.

**Examples:**
```piscript
print(is_upper("HELLO WORLD"))   // Output: true
print(is_upper("Hello World"))   // Output: false
print(is_upper("123!@#"))        // Output: true
print(is_upper(""))              // Output: true
```
---
### is_lower(string)

**Description:**  
Checks if **all alphabetic characters** in the given `string` are lowercase.

**Arguments:**
- `string` *(string)* – The string to check.

**Returns:**
- *(bool)* – `true` if every alphabetic character is lowercase or if the string contains no alphabetic characters; otherwise, `false`.

**Behavior:**
- Non-alphabetic characters (digits, punctuation, whitespace) are ignored.
- An empty string or a string without alphabetic characters returns `true`.

**Examples:**
```piscript
print(is_lower("hello world"))   // Output: true
print(is_lower("Hello World"))   // Output: false
print(is_lower("123!@#"))        // Output: true
print(is_lower(""))              // Output: true
```
---
### is_digit(string)

**Description:**  
Checks if the entire `string` consists **only of digit characters** (`0`–`9`).

**Arguments:**
- `string` *(string)* – The string to check.

**Returns:**
- *(bool)* – `true` if every character in the string is a digit; otherwise, `false`.

**Behavior:**
- An empty string returns `false` because it contains no digits.
- Non-digit characters cause the function to return `false`.

**Examples:**
```piscript
print(is_digit("123456"))     // Output: true
print(is_digit("123a456"))    // Output: false
print(is_digit(""))           // Output: false
print(is_digit("007"))        // Output: true
```
---
### is_numeric(string)

**Description:**  
Checks if the given `string` represents a **valid numeric value**, including integers and decimals.

**Arguments:**
- `string` *(string)* – The string to check.

**Returns:**
- *(bool)* – `true` if the string represents a valid number; otherwise, `false`.

**Behavior:**
- Accepts optional leading `+` or `-` signs.
- Accepts decimal points (e.g., `"123.45"`).
- Does not allow scientific notation (`e` or `E`).
- Whitespace or other non-numeric characters cause `false`.
- Empty string returns `false`.

**Examples:**
```piscript
print(is_numeric("123"))       // Output: true
print(is_numeric("-123.45"))   // Output: true
print(is_numeric("+0.99"))     // Output: true
print(is_numeric("12a3"))      // Output: false
print(is_numeric(""))          // Output: false
```
---
### is_alpha(string)

**Description:**  
Checks if the entire `string` consists **only of alphabetic characters** (`A`–`Z` and `a`–`z`).

**Arguments:**
- `string` *(string)* – The string to check.

**Returns:**
- *(bool)* – `true` if every character in the string is alphabetic; otherwise, `false`.

**Behavior:**
- An empty string returns `false`.
- Non-alphabetic characters (digits, punctuation, whitespace) cause the function to return `false`.

**Examples:**
```piscript
print(is_alpha("Hello"))       // Output: true
print(is_alpha("Hello123"))    // Output: false
print(is_alpha(" "))           // Output: false
print(is_alpha(""))            // Output: false

```
---
### is_alnum(string)

**Description:**  
Checks if the entire `string` consists **only of alphanumeric characters** — letters (`A`–`Z`, `a`–`z`) and digits (`0`–`9`).

**Arguments:**
- `string` *(string)* – The string to check.

**Returns:**
- *(bool)* – `true` if every character in the string is alphanumeric; otherwise, `false`.

**Behavior:**
- An empty string returns `false`.
- Any non-alphanumeric character (such as spaces, punctuation, symbols) causes the function to return `false`.

**Examples:**
```piscript
print(is_alnum("Hello123"))   // Output: true
print(is_alnum("Hello 123"))  // Output: false (space is not alphanumeric)
print(is_alnum("123456"))     // Output: true
print(is_alnum(""))           // Output: false
```
---
