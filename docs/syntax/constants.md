# Built-in Constants

## Math and Screen

| Constant | Meaning |
| --- | --- |
| `PI` | Pi |
| `E` | Euler's number |
| `WIDTH` | Virtual screen width, `128` |
| `HEIGHT` | Virtual screen height, `128` |

## Audio Waveforms

| Constant | Meaning |
| --- | --- |
| `WAVE_SINE` | Sine oscillator type for `tone` and `melody` |
| `WAVE_SQUARE` | Square oscillator type |
| `WAVE_TRIANGLE` | Triangle oscillator type |
| `WAVE_NOISE` | Noise oscillator type |

## Colors

PiScript exposes every default palette slot as a `COLOR_*` constant. Color
arguments also accept numeric palette indices from `0` to `79` or packed
`0xAARRGGBB` numbers where the drawing API accepts a color.

The base group is:

```text
COLOR_SOFT_BLACK          COLOR_DEEP_BLUE
COLOR_DARK_MAGENTA        COLOR_DEEP_GREEN
COLOR_WARM_BROWN          COLOR_CHARCOAL_GRAY
COLOR_COOL_LIGHT_GRAY     COLOR_SOFT_IVORY
COLOR_VIVID_RED           COLOR_AMBER_ORANGE
COLOR_WARM_YELLOW         COLOR_NEON_GREEN
COLOR_SKY_BLUE            COLOR_DUSTY_PURPLE
COLOR_PINK_ROSE           COLOR_LIGHT_PEACH
COLOR_DARK_COCOA          COLOR_MIDNIGHT_BLUE
COLOR_PLUM                COLOR_OCEAN_TEAL
COLOR_BRICK_RED           COLOR_MUTED_MAUVE
COLOR_WARM_STONE          COLOR_PALE_YELLOW
COLOR_DEEP_PINK           COLOR_ORANGE_RED
COLOR_LIME_YELLOW         COLOR_EMERALD
COLOR_BRIGHT_COBALT       COLOR_DUSTY_VIOLET
COLOR_CORAL               COLOR_SOFT_SALMON
```

Additional palette groups use `COLOR_GB_*`, `COLOR_NES_*`, and `COLOR_SEGA_*`
names. Use `palette(index)` to read any default palette entry as a packed color.

```piscript
clear(COLOR_SOFT_BLACK)
rect(8, 8, 40, 20, COLOR_NEON_GREEN, true)
pixel(64, 64, 0x80ff00ff)
draw()
```
