# Media Functions

Media functions cover loaded images, generated or file-backed audio, and the
small OBJ-based 3D path.

## Images and 2D Transforms

2D transforms accept images. They return transformed image values rather than
drawing immediately.

### `image(path)`

Loads an image file and converts pixels into PiScript image data with alpha.

```piscript
let background = image("assets/background.png")
rend2d(background, 0, 0)
draw()
```

### `crop(image, x, y, width, height)`

Copies a rectangle from the source. Destination pixels sourced from outside the
input become transparent.

```piscript
let sheet = image("assets/tiles.png")
let tile = crop(sheet, 0, 0, 16, 16)
rend2d(tile, 8, 8)
draw()
```

### `resize(image, width, height)`

Resizes with nearest-neighbor sampling.

```piscript
let portrait = image("assets/face.png")
let tiny = resize(portrait, 24, 24)
rend2d(tiny, 0, 0)
draw()
```

### `scale2d(image, sx, sy)`

Scales by positive X and Y scale factors.

```piscript
let player = image("assets/player.png")
let big = scale2d(player, 2, 2)
rend2d(big, 48, 48)
draw()
```

### `tran2d(image, dx, dy)`

Moves source pixels inside an output of the same dimensions. Pixels shifted out
are discarded and newly uncovered pixels are transparent.

```piscript
let moved = tran2d(image("assets/icon.png"), 2, -1)
rend2d(moved, 10, 10)
draw()
```

### `flip(image, flip_x[, flip_y])`

Mirrors horizontally and optionally vertically.

```piscript
let right = image("assets/player-right.png")
let left = flip(right, true)
rend2d(left, 40, 60)
draw()
```

### `rot2d(image, degrees)`

Rotates around the image center and returns an output with the same dimensions.
Pixels that rotate out of bounds are lost.

```piscript
let needle = image("assets/needle.png")
rend2d(rot2d(needle, 45), 56, 56)
draw()
```

### `copy2d(image)`

Deep-copies the pixel and alpha buffers.

```piscript
let original = image("assets/tile.png")
let duplicate = copy2d(original)
rend2d(duplicate, 20, 20)
draw()
```

### `rend2d(image[, x, y])`

Draws image pixels to the screen, respecting transparency.

```piscript
let logo = image("assets/logo.png")
clear(COLOR_SOFT_BLACK)
rend2d(logo, 12, 12)
draw()
```

## Audio

Audio constructors produce sound objects. Playback control functions operate on
those sound objects.

### `sound(index_or_path)`

With a number, loads sound data from a cartridge sound index. With a string,
loads an audio file path.

```piscript
let hit = sound("assets/hit.wav")
play(hit)
```

### `tone(samples)`

Builds a sound from raw sample amplitudes in the range the generator clamps to
`-1..1`.

```piscript
let click = tone([0, 1, -1, 0])
play(click)
```

### `tone(frequency, duration_ms, waveform)`

Generates a tone with an oscillator waveform such as `WAVE_SINE` or
`WAVE_SQUARE`.

```piscript
let beep = tone(440, 120, WAVE_SINE)
play(beep)
```

### `melody(notes)`

Builds one sound from a flat list of frequency, duration, waveform triples.

```piscript
let tune = melody([
    440, 120, WAVE_SQUARE,
    660, 120, WAVE_SQUARE
])
play(tune)
```

### `play(sound[, loop[, channel[, start[, length]]]])`

Starts playback. `loop` overrides the sound's default loop flag. `channel`
chooses a mixer channel or leaves selection automatic with `-1`. `start` and
`length` currently apply to cartridge-index sounds.

```piscript
let music = sound("assets/loop.wav")
play(music, true)
```

### `stop(sound)`

Halts the sound object's active channel and clears its playback state.

```piscript
stop(music)
```

### `pause(sound)`

Pauses an active sound channel.

```piscript
pause(music)
```

### `resume(sound)`

Resumes a paused sound channel.

```piscript
resume(music)
```

### `is_playing(sound)`

Returns whether the sound object's mixer channel is currently playing.

```piscript
if is_playing(music)
    writeln("music is active")
```

### `channel(sound)`

Returns the assigned mixer channel or `-1` if the sound has no assigned
channel.

```piscript
writeln(channel(music))
```

### `set_loop(sound, bool)`

Sets the sound's default looping flag for later `play` calls.

```piscript
set_loop(music, true)
play(music)
```

## 3D Models

The 3D path loads OBJ triangles, transforms model data, projects to screen
coordinates, renders triangles into the framebuffer, then uses `draw()`.

### `load3d(path[, texture_image])`

Loads a Wavefront OBJ model. A second argument may supply a PiScript image
texture for textured faces.

```piscript
let ship = load3d("assets/ship.obj")
```

### `rot3d(model, x_degrees, y_degrees, z_degrees)`

Returns a rotated model value.

```piscript
ship = rot3d(ship, 0, 25, 0)
```

### `tran3d(model, tx, ty, tz)`

Returns a translated model value.

```piscript
ship = tran3d(ship, 0, 0, 3)
```

### `scale3d(model, sx, sy, sz)`

Returns a scaled model value.

```piscript
ship = scale3d(ship, 1.5, 1.5, 1.5)
```

### `proj3d(model, fov, camera_z)`

Projects 3D model positions into 128x128 screen space and computes triangle
brightness.

```piscript
let screen_ship = proj3d(ship, 90, 0)
```

### `rend3d(model[, color[, filled]])`

Renders a projected model. Without `filled`, output is wireframe. Filled
rendering uses texture data if the model has texture input, otherwise it fills
triangles with color and lighting.

```piscript
clear(COLOR_SOFT_BLACK)
rend3d(screen_ship, COLOR_SKY_BLUE, true)
draw()
```
