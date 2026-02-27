from PIL import Image, ImageDraw
import imageio
import sys
import os

def add_rounded_corners(frame, radius):
    """
    Adds rounded corners to a single frame (PIL Image).
    """
    frame = frame.convert("RGBA")
    width, height = frame.size

    # Create rounded mask
    mask = Image.new("L", (width, height), 0)
    draw = ImageDraw.Draw(mask)
    draw.rounded_rectangle(
        [(0, 0), (width, height)],
        radius=radius,
        fill=255
    )

    # Apply mask to frame
    rounded = Image.new("RGBA", (width, height))
    rounded.paste(frame, (0, 0), mask=mask)

    return rounded


def round_gif(input_path, output_path, radius=20):
    """
    Applies rounded corners to all frames of a GIF.
    """
    gif = Image.open(input_path)

    frames = []
    durations = []

    try:
        while True:
            frame = gif.copy()
            duration = gif.info.get("duration", 40)

            rounded_frame = add_rounded_corners(frame, radius)

            frames.append(rounded_frame)
            durations.append(duration)

            gif.seek(gif.tell() + 1)

    except EOFError:
        pass

    # Save new GIF
    frames[0].save(
        output_path,
        save_all=True,
        append_images=frames[1:],
        duration=durations,
        loop=0,
        disposal=2,
        transparency=0
    )

    print(f"✅ Rounded GIF saved as: {output_path}")


if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python round_gif_corners.py input.gif output.gif [radius]")
        sys.exit(1)

    input_gif = sys.argv[1]
    output_gif = sys.argv[2]
    radius = int(sys.argv[3]) if len(sys.argv) > 3 else 30

    round_gif(input_gif, output_gif, radius)