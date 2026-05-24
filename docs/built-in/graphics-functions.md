# Graphics Functions

Graphics built-ins draw into a virtual 128x128 framebuffer. Drawing functions
change the pixel buffer immediately, but the window changes when `draw()`
presents the frame.

Color arguments accept:

- a palette index from `0` to `79`
- a named `COLOR_*` constant
- a packed `0xAARRGGBB` number where supported

## Pixels and Shapes

### `pixel(x, y, color[, alpha])`

Sets one screen pixel. `alpha`, when supplied, is an additional opacity value
used by the alpha pixel path.

```piscript
pixel(64, 64, COLOR_SKY_BLUE)
pixel(65, 64, 0x80ff00ff, 0.5)
draw()
```

### `line(x1, y1, x2, y2, color)`

Draws a one-pixel-wide line between two screen coordinates.

```piscript
clear(COLOR_SOFT_BLACK)
line(0, 0, WIDTH - 1, HEIGHT - 1, COLOR_NEON_GREEN)
draw()
```

### `rect(x, y, width, height, color[, filled])`

Draws a rectangle at a top-left position. `filled` defaults to false.

```piscript
rect(8, 8, 32, 20, COLOR_CORAL)
rect(48, 8, 32, 20, COLOR_SKY_BLUE, true)
draw()
```

### `circ(x, y, radius, color[, filled])`

Draws an outline or filled circle.

```piscript
circ(64, 64, 18, COLOR_WARM_YELLOW)
circ(64, 64, 6, COLOR_VIVID_RED, true)
draw()
```

### `poly(points, color[, filled])`

Draws a polygon from a list of point lists. The drawing layer consumes the
point coordinates and either outlines or fills the polygon.

```piscript
let tri = [[20, 20], [80, 30], [50, 90]]
poly(tri, COLOR_EMERALD, true)
draw()
```

## Frame Control

### `clear([color])`

Fills the screen buffer with one color. If omitted, the runtime uses its default
clear color.

```piscript
clear(COLOR_SOFT_BLACK)
draw()
```

### `draw([offset_x, offset_y])`

Presents the current framebuffer. Optional offsets set the global draw offset
used by screen drawing.

```piscript
clear(COLOR_SOFT_BLACK)
pixel(1, 1, COLOR_SOFT_IVORY)
draw()
```

### `camera([x, y])`

Sets the global draw offset used by subsequent drawing calls. Calling
`camera()` with no arguments resets the camera to `(0, 0)`.

```piscript
camera(player_x - WIDTH / 2, player_y - HEIGHT / 2)
rend2d(level, 0, 0)
camera()
draw()
```

## Reading Colors

### `color(x, y)`

Reads a screen pixel and returns its closest default palette index.

```piscript
pixel(10, 10, COLOR_CORAL)
writeln(color(10, 10))
```

### `palette(index)`

Reads default palette slot `0..79` and returns a packed `0xAARRGGBB` number.

```piscript
let packed = palette(12)
rect(2, 2, 20, 10, packed, true)
draw()
```
