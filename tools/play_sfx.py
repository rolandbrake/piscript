#!/usr/bin/env python3
import numpy as np
import struct
import argparse
import sounddevice as sd


# -----------------------------
# Delta decode (SAFE)
# -----------------------------
def delta_decode(delta):
    out = np.zeros(len(delta), dtype=np.int32)

    out[0] = int(delta[0])

    for i in range(1, len(delta)):
        out[i] = out[i - 1] + int(delta[i])

    return np.clip(out, -32768, 32767).astype(np.int16)


# -----------------------------
# Load file
# -----------------------------
def load(path):
    with open(path, "rb") as f:
        data = f.read()

    sr = struct.unpack("<I", data[0:4])[0]
    count = struct.unpack("<I", data[4:8])[0]
    mode = data[8]

    payload = data[9:]

    if mode == 0:
        samples = np.frombuffer(payload, dtype=np.int16)
    else:
        delta = np.frombuffer(payload, dtype=np.int16)
        samples = delta_decode(delta)

    return sr, samples


# -----------------------------
# Main
# -----------------------------
def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("input")
    args = parser.parse_args()

    sr, samples = load(args.input)

    print(f"[ok] sample rate: {sr}")
    print(f"[ok] samples: {len(samples)}")

    audio = samples.astype(np.float32) / 32768.0

    sd.play(audio, sr)
    sd.wait()


if __name__ == "__main__":
    main()