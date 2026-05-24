# Graphics

Graphics are the center of PiScript's runtime. Screen text, pixels, shapes,
images, and projected 3D output all write into the same virtual framebuffer.
`draw()` presents that buffer as a visible frame.

## Screen Size and Coordinates

The screen is `128` pixels wide and `128` pixels tall. `WIDTH` and `HEIGHT`
expose those values to scripts, so code can describe the screen without
repeating the numbers.

Coordinates start at the top-left corner:

- `(0, 0)` is the first pixel.
- X increases to the right.
- Y increases downward.
- The last on-screen pixel is `(WIDTH - 1, HEIGHT - 1)`, which is `(127, 127)`.

```piscript
clear(COLOR_SOFT_BLACK)
line(0, 0, WIDTH - 1, HEIGHT - 1, COLOR_NEON_GREEN)
pixel(WIDTH / 2, HEIGHT / 2, COLOR_WARM_YELLOW)
draw()
```

## Colors and Palette

Most drawing calls accept a color as a default palette index from `0` to `79`,
a named `COLOR_*` constant, or a packed `0xAARRGGBB` number where that API
supports packed color. Packed colors store alpha, red, green, blue bytes in
that order.

`palette(index)` reads a default palette slot as a packed color. `color(x, y)`
reads a screen pixel and returns its closest default palette index.

```piscript
let accent = palette(12)

clear(COLOR_SOFT_BLACK)
rect(8, 8, 48, 24, accent, true)
pixel(64, 20, 0xffff00ff)
draw()
```

The named color constants are listed in [Constants](constants.md).

## Framebuffer and Drawing Process

PiScript drawing is buffered:

1. `clear(color)` fills the framebuffer for the next frame.
2. Drawing calls such as `pixel`, `line`, `rect`, `circ`, `poly`, `rend2d`,
   `rend3d`, `print`, `println`, and `printf` change that framebuffer.
3. `draw()` presents the current framebuffer to the window.

Without `draw()`, pixels and characters can be prepared in memory but are not
shown as the new frame. Screen examples should end with `draw()` after their
visible drawing work.

```piscript
clear(COLOR_SOFT_BLACK)
rect(10, 10, 40, 18, COLOR_CORAL, true)
println("READY", 16, 16, COLOR_SOFT_IVORY)
draw()
```

`clear()` is normally called at the start of a moving frame. Leaving it out
keeps previous pixels in the buffer, which can be useful for trails or
incremental drawing.

## Screen Text

`print`, `println`, and `printf` are graphics APIs. They draw characters into
the 128x128 screen buffer at the screen cursor or at coordinates passed to the
call. They do not write terminal output.

Use `writeln` for the host console. Use `cursor(x, y[, color])` to move the
screen text cursor and set the color for later screen text calls.

```piscript
clear(COLOR_SOFT_BLACK)
cursor(4, 4, COLOR_WARM_YELLOW)
println("score")
printf("{0}", 120)
draw()
```

## Event Loop

Interactive PiScript programs usually draw inside a loop. A frame commonly
does this work in order:

1. Read input with APIs such as `key`, `typed`, or `mouse`.
2. Update program state.
3. `clear` the frame when old pixels should not remain.
4. Draw pixels, shapes, images, or screen text.
5. Call `draw()` so the buffer becomes the visible frame.
6. Call `sleep(milliseconds)` when the loop needs an intentional delay.

```piscript
let x = WIDTH / 2

while true {
    if key("LEFT")
        x -= 1
    if key("RIGHT")
        x += 1

    clear(COLOR_SOFT_BLACK)
    circ(x, HEIGHT / 2, 5, COLOR_SKY_BLUE, true)
    println("fps " + fps(), 2, 2, COLOR_SOFT_IVORY)
    draw()
    sleep(16)
}
```

`key(name[, once])` checks keyboard state. With `once = true`, a key press is
reported once at its pressed transition. `typed()` returns typed character
input, and `mouse()` returns `[x, y]` in screen coordinates.

## Core Graphics API

| API | Role |
| --- | --- |
| `clear([color])` | Fill the current framebuffer. |
| `draw([offset_x, offset_y])` | Present the framebuffer. |
| `camera([x, y])` | Set or reset the global draw offset. |
| `pixel(x, y, color[, alpha])` | Draw one pixel. |
| `line(x1, y1, x2, y2, color)` | Draw a line. |
| `rect(x, y, width, height, color[, filled])` | Draw a rectangle. |
| `circ(x, y, radius, color[, filled])` | Draw a circle. |
| `poly(points, color[, filled])` | Draw a polygon from point lists. |
| `print`, `println`, `printf` | Draw characters into the screen buffer. |
| `cursor(x, y[, color])` | Move the screen text cursor. |
| `palette(index)`, `color(x, y)` | Read palette and framebuffer colors. |
| `key`, `typed`, `mouse` | Read interaction state for the loop. |
| `time`, `sleep`, `fps` | Measure, pace, and inspect frame timing. |

Detailed signatures and examples live in [Graphics Functions](../built-in/graphics-functions.md),
[I/O Functions](../built-in/io-functions.md), [Time Functions](../built-in/time-functions.md),
and [System Functions](../built-in/system-functions.md).
