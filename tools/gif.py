from PIL import Image, ImageDraw, ImageSequence
import sys
import os


def add_border_radius_to_frame(frame, border_size, border_color, radius):
    # Convert frame to RGBA
    frame = frame.convert("RGBA")

    width, height = frame.size

    # New size including border
    new_width = width + 2 * border_size
    new_height = height + 2 * border_size

    # Create background with border color
    background = Image.new("RGBA", (new_width, new_height), border_color)

    # Create rounded mask
    mask = Image.new("L", (new_width, new_height), 0)
    draw = ImageDraw.Draw(mask)
    draw.rounded_rectangle(
        [(0, 0), (new_width, new_height)],
        radius=radius,
        fill=255
    )

    # Paste original image onto background
    background.paste(frame, (border_size, border_size), frame)

    # Apply rounded mask
    background.putalpha(mask)

    return background


def process_gif(input_path, output_path, border_size=20, border_color=(255, 0, 0, 255), radius=40):
    gif = Image.open(input_path)

    frames = []
    durations = []

    for frame in ImageSequence.Iterator(gif):
        processed = add_border_radius_to_frame(frame, border_size, border_color, radius)
        frames.append(processed)
        durations.append(frame.info.get("duration", 100))

    # Save animated GIF
    frames[0].save(
        output_path,
        save_all=True,
        append_images=frames[1:],
        duration=durations,
        loop=gif.info.get("loop", 0),
        disposal=2,
        transparency=0
    )


if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python script.py input.gif output.gif")
        sys.exit(1)

    input_gif = sys.argv[1]
    output_gif = sys.argv[2]

    process_gif(
        input_gif,
        output_gif,
        border_size=20,                 # Change border thickness
        border_color=(0, 0, 0, 255),     # Change border color (RGBA)
        radius=40                       # Change corner radius
    )

    print("Done!")
