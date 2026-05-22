#!/usr/bin/env python3
from PIL import Image
import numpy as np
import argparse
import struct

palette = np.array([
    (2, 4, 6), (34, 48, 96), (134, 42, 90), (4, 140, 90),
    (176, 88, 60), (100, 92, 84), (200, 202, 206), (255, 244, 236),
    (255, 12, 90), (255, 168, 12), (255, 238, 48), (12, 232, 64),
    (48, 180, 255), (138, 124, 164), (255, 126, 176), (255, 210, 176),

    (46, 28, 24), (22, 36, 60), (72, 38, 60), (24, 90, 96),
    (124, 54, 48), (80, 58, 66), (168, 142, 128), (246, 242, 136),
    (198, 26, 92), (255, 116, 44), (176, 236, 56), (4, 188, 80),
    (12, 96, 190), (124, 78, 110), (255, 120, 96), (255, 164, 140),

    (208, 208, 88), (160, 168, 64), (112, 128, 40), (64, 80, 16),
    (51, 44, 80), (70, 135, 143), (148, 227, 68), (226, 243, 228),
    (33, 30, 32), (85, 85, 104), (160, 160, 139), (233, 239, 236),
    (124, 63, 88), (235, 107, 111), (249, 168, 117), (255, 246, 211),

    (194, 198, 48), (63, 161, 48), (78, 88, 23), (52, 43, 25),
    (255, 211, 123), (225, 135, 52), (195, 78, 41), (99, 72, 5),
    (193, 61, 149), (141, 21, 35), (255, 255, 255), (173, 173, 203),
    (89, 110, 120), (114, 179, 255), (61, 101, 182), (36, 68, 73),

    (43, 18, 13), (159, 18, 17), (252, 20, 0), (252, 106, 0),
    (252, 252, 0), (156, 12, 156), (255, 9, 157), (0, 7, 44),
    (0, 0, 255), (103, 205, 252), (0, 72, 73), (0, 201, 8),
    (82, 255, 0), (173, 89, 80), (252, 180, 72), (221, 217, 230)
], dtype=np.uint8)


def rle_decode(data, expected_size):
    decoded = np.empty(expected_size, dtype=np.uint8)
    i = 0
    j = 0

    while i < len(data):
        value = data[i]
        count = data[i + 1]
        decoded[j:j+count] = value
        j += count
        i += 2

    return decoded


def load_sprite(path):
    with open(path, "rb") as f:
        data = f.read()

    # Read header
    w = data[0] | (data[1] << 8)
    h = data[2] | (data[3] << 8)

    compression = data[4]
    original_size = struct.unpack("<I", data[5:9])[0]

    payload = data[9:]

    if compression == 0:
        flat = np.frombuffer(payload, dtype=np.uint8)
    elif compression == 1:
        flat = rle_decode(payload, original_size)
    else:
        raise ValueError("Unknown compression type")

    pixels = flat.reshape(h, w)
    return w, h, pixels


def main():
    parser = argparse.ArgumentParser(description="Display VM sprite (with RLE support)")
    parser.add_argument("input", help="Input .bin file")
    parser.add_argument("-s", "--scale", type=int, default=1, help="Scale factor")
    parser.add_argument("--save", help="Save as PNG")

    args = parser.parse_args()

    w, h, indices = load_sprite(args.input)

    img = Image.fromarray(palette[indices], "RGB")

    if args.scale > 1:
        img = img.resize((w * args.scale, h * args.scale), Image.NEAREST)

    print(f"[ok] Loaded: {args.input} ({w}x{h})")

    img.show()

    if args.save:
        img.save(args.save)
        print(f"[ok] Saved: {args.save}")


if __name__ == "__main__":
    main()