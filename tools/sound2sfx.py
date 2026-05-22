#!/usr/bin/env python3
import numpy as np
import argparse
import struct
from scipy.io import wavfile


TARGET_SR = 44100  # safe playback rate


# -----------------------------
# Resample (simple linear)
# -----------------------------
def resample(data, orig_sr, target_sr):
    if orig_sr == target_sr:
        return data

    ratio = target_sr / orig_sr
    new_len = int(len(data) * ratio)

    return np.interp(
        np.linspace(0, len(data), new_len, endpoint=False),
        np.arange(len(data)),
        data
    ).astype(np.float32)


# -----------------------------
# Delta encode
# -----------------------------
def delta_encode(samples):
    delta = np.empty(len(samples), dtype=np.int16)
    delta[0] = samples[0]
    delta[1:] = samples[1:] - samples[:-1]
    return delta


# -----------------------------
# Convert
# -----------------------------
def convert(input_path, output_path):
    sr, data = wavfile.read(input_path)

    # mono
    if len(data.shape) > 1:
        data = data.mean(axis=1)

    # normalize float
    data = data.astype(np.float32)
    data /= np.max(np.abs(data)) + 1e-6

    # resample to stable rate
    data = resample(data, sr, TARGET_SR)
    sr = TARGET_SR

    # convert to int16 PCM
    pcm = (data * 32767).astype(np.int16)

    # delta encode
    delta = delta_encode(pcm)

    raw_bytes = pcm.tobytes()
    delta_bytes = delta.tobytes()

    use_delta = len(delta_bytes) < len(raw_bytes)

    with open(output_path, "wb") as f:
        f.write(struct.pack("<I", sr))
        f.write(struct.pack("<I", len(pcm)))

        if use_delta:
            f.write(struct.pack("B", 1))
            f.write(delta_bytes)
            method = "delta"
        else:
            f.write(struct.pack("B", 0))
            f.write(raw_bytes)
            method = "raw"

    print(f"[ok] {input_path} -> {output_path}")
    print(f"     samples: {len(pcm)}")
    print(f"     method: {method}")


# -----------------------------
# CLI
# -----------------------------
def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("input")
    parser.add_argument("-o", "--output", default="out.snd")

    args = parser.parse_args()
    convert(args.input, args.output)


if __name__ == "__main__":
    main()