#!/usr/bin/env python3
from PIL import Image
import argparse
import os
import sys


def positive_int(value):
    try:
        number = int(value)
    except ValueError:
        raise argparse.ArgumentTypeError("must be an integer")

    if number <= 0:
        raise argparse.ArgumentTypeError("must be greater than zero")

    return number


def output_path_for(input_path):
    directory, filename = os.path.split(input_path)
    name, ext = os.path.splitext(filename)
    if not ext:
        ext = ".png"
    return os.path.join(directory, f"{name}_resized{ext}")


def calculate_size(original_w, original_h, width, height):
    if width is None and height is None:
        raise ValueError("provide --width, --height, or both")

    if width is not None and height is not None:
        original_ratio = original_w / original_h
        new_ratio = width / height

        if abs(original_ratio - new_ratio) > 0.001:
            print(
                "[warning] aspect ratio will change "
                f"({original_w}x{original_h} -> {width}x{height})",
                file=sys.stderr,
            )

        return width, height

    if width is not None:
        height = round(original_h * (width / original_w))
        return width, max(1, height)

    width = round(original_w * (height / original_h))
    return max(1, width), height


def save_resized_image(img, output_path, source_format):
    extension = os.path.splitext(output_path)[1].lower()
    save_args = {}

    if extension in (".jpg", ".jpeg"):
        if img.mode in ("RGBA", "LA", "P"):
            img = img.convert("RGB")
        save_args = {
            "quality": 85,
            "optimize": True,
            "progressive": True,
        }
    elif extension == ".png":
        save_args = {
            "optimize": True,
            "compress_level": 9,
        }
    elif extension == ".webp":
        save_args = {
            "quality": 85,
            "method": 6,
        }
    elif source_format:
        save_args = {"format": source_format}

    img.save(output_path, **save_args)


def format_bytes(size):
    for unit in ("B", "KB", "MB", "GB"):
        if size < 1024:
            return f"{size:.1f} {unit}" if unit != "B" else f"{size} {unit}"
        size /= 1024
    return f"{size:.1f} TB"


def resize_image(input_path, output_path, width, height, nearest):
    with Image.open(input_path) as img:
        original_size = os.path.getsize(input_path)
        source_format = img.format
        new_w, new_h = calculate_size(img.width, img.height, width, height)
        resample = Image.NEAREST if nearest else Image.LANCZOS
        resized = img.resize((new_w, new_h), resample)
        save_resized_image(resized, output_path, source_format)

    output_size = os.path.getsize(output_path)

    print(f"[ok] {input_path} -> {output_path}")
    print(f"     size: {new_w}x{new_h}")
    print(f"     file: {format_bytes(original_size)} -> {format_bytes(output_size)}")

    if output_size > original_size:
        print(
            "[warning] output file is larger than the original. "
            "This can happen when the source image was already highly compressed "
            "or when changing formats.",
            file=sys.stderr,
        )


def main():
    parser = argparse.ArgumentParser(
        description="Resize an image, preserving aspect ratio when only one dimension is provided"
    )
    parser.add_argument("input", help="Input image file")
    parser.add_argument("-o", "--output", help="Output image file")
    parser.add_argument("-w", "--width", type=positive_int, help="New width")
    parser.add_argument("-H", "--height", type=positive_int, help="New height")
    parser.add_argument(
        "--nearest",
        action="store_true",
        help="Use nearest-neighbor scaling, useful for pixel art",
    )

    args = parser.parse_args()
    output = args.output or output_path_for(args.input)

    try:
        resize_image(args.input, output, args.width, args.height, args.nearest)
    except ValueError as error:
        parser.error(str(error))


if __name__ == "__main__":
    main()
