#!/usr/bin/env python3
from PIL import Image
import numpy as np
import os
import struct
import argparse

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
], dtype=np.int32)


def map_to_palette(img):
    pixels = img.reshape(-1, 3).astype(np.int32)
    diff = pixels[:, None, :] - palette[None, :, :]
    dist = np.sum(diff * diff, axis=2)
    return np.argmin(dist, axis=1).astype(np.uint8)


def rle_encode(data):
    if len(data) == 0:
        return bytearray()

    encoded = bytearray()
    prev = data[0]
    count = 1

    for b in data[1:]:
        if b == prev and count < 255:
            count += 1
        else:
            encoded.append(prev)
            encoded.append(count)
            prev = b
            count = 1

    encoded.append(prev)
    encoded.append(count)

    return encoded


def convert(input_path, output_path=None):
    img = Image.open(input_path).convert("RGB")
    img_np = np.array(img, dtype=np.uint8)

    h, w, _ = img_np.shape

    indices = map_to_palette(img_np).reshape(h, w)
    flat = indices.flatten()

    raw_data = bytearray(flat)
    compressed = rle_encode(flat)

    data = bytearray()

    # Header: width + height
    data.extend(struct.pack("<H", w))
    data.extend(struct.pack("<H", h))

    # Choose best storage method
    if len(compressed) < len(raw_data):
        data.append(1)  # RLE
        data.extend(struct.pack("<I", len(raw_data)))  # original size
        data.extend(compressed)
        method = "RLE"
        final_size = len(compressed)
    else:
        data.append(0)  # RAW
        data.extend(struct.pack("<I", len(raw_data)))
        data.extend(raw_data)
        method = "RAW"
        final_size = len(raw_data)

    if output_path is None:
        base = os.path.splitext(os.path.basename(input_path))[0]
        output_path = base + ".bin"

    with open(output_path, "wb") as f:
        f.write(data)

    print(f"[ok] {input_path} -> {output_path}")
    print(f"     size: {w}x{h}")
    print(f"     method: {method}")
    print(f"     data size: {final_size} bytes")


def main():
    parser = argparse.ArgumentParser(description="Convert image to VM sprite format (with RLE)")
    parser.add_argument("input", help="Input image file")
    parser.add_argument("-o", "--output", help="Output .bin file")

    args = parser.parse_args()
    convert(args.input, args.output)


if __name__ == "__main__":
    main()