# I/O Functions

PiScript has two text output paths. `print`, `println`, and `printf` draw text
into the 128x128 screen buffer. `writeln` writes a line to the host console.

## Screen Text

### `print(text[, x, y[, color]])`

Draws `text` at the current screen cursor position. Supplying `x` and `y`
places that call directly. The optional fourth argument changes the text color
for that draw call.

```piscript
clear(COLOR_SOFT_BLACK)
print("HP", 2, 2, COLOR_WARM_YELLOW)
draw()
```

`print` does not move to a new text row automatically.

### `println([text[, x, y[, color]]])`

Draws text like `print`, then advances the screen cursor to the next text row.
With no text argument it only advances the cursor.

```piscript
println("line one", 2, 2, COLOR_SOFT_IVORY)
println("line two")
draw()
```

### `printf(format, value...)`

Draws formatted screen text. `{0}` inserts the first formatting value, `{1}`
the second, and so on. A placeholder such as `{0:7}` draws that inserted value
with a palette color. `\n` moves the screen cursor down.

```piscript
printf("score {0:10}\nhp {1}", score, hp)
draw()
```

## Console Text

### `writeln(message[, flag])`

Converts `message` to text and writes a console line. Flag `"e"` writes with
error coloring and flag `"w"` warning coloring where the terminal supports it.

```piscript
writeln("loading level 1")
writeln("missing optional save", "w")
```

## Keyboard and Text Input

### `key(name_or_code[, once])`

Returns whether a keyboard scancode is pressed. The first argument may be a
number or a name such as `"A"`, `"SPACE"`, `"ENTER"`, `"ESC"`, `"UP"`,
`"DOWN"`, `"LEFT"`, `"RIGHT"`, `"LSHIFT"`, or `"LCTRL"`. Letter and number
names can also use a `KEY_` prefix.

When `once` is true, the function returns true only on the transition from not
pressed to pressed for that key.

```piscript
if key("LEFT")
    x -= 1

if key("SPACE", true)
    fire()
```

### `typed()`

Returns text input events collected from SDL as a string. This is for character
typing rather than key-state polling.

```piscript
let letters = typed()
if !empty(letters)
    writeln("typed " + letters)
```

### `input(prompt)`

Prints a prompt on the console and reads one line from standard input. The line
is returned without its trailing newline.

```piscript
let name = input("name: ")
writeln("hello " + name)
```

## Files

### `open(path[, mode])`

Opens a file and returns a file object. Mode defaults to `"r"` and follows the
host C file-mode strings supported by the runtime.

```piscript
let save = open("save.txt", "w")
```

### `read(file)`

Reads remaining bytes from the current file position and returns them as a
string.

```piscript
let f = open("notes.txt")
let content = read(f)
close(f)
writeln(content)
```

### `write(file, text)`

Writes a string at the file's current position and returns `true` when the
write succeeds.

```piscript
let f = open("score.txt", "w")
write(f, "42")
close(f)
```

### `seek(file, position)`

Moves the file position to a byte offset from the beginning of the file and
returns `true` on success.

```piscript
let f = open("notes.txt")
seek(f, 0)
writeln(read(f))
close(f)
```

### `close(file)`

Closes an open file object and marks it closed. Other file operations on that
handle then raise runtime errors.

```piscript
let f = open("log.txt", "w")
write(f, "done")
close(f)
```
