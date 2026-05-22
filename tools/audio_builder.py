import numpy as np
import soundfile as sf
import sounddevice as sd
import struct
import argparse
import os

DEFAULT_SAMPLE_RATE = 22050

# ----------------------------
# CONVERT WAV → PCM
# ----------------------------
def convert_wav(input_path, target_sr, normalize=True):
    data, sample_rate = sf.read(input_path)

    if len(data.shape) > 1:
        data = data.mean(axis=1)

    if normalize:
        max_val = np.max(np.abs(data))
        if max_val > 0:
            data = data / max_val

    ratio = sample_rate / target_sr
    indices = (np.arange(0, len(data), ratio)).astype(int)
    resampled = data[indices]

    pcm = (resampled * 32767).astype(np.int16)
    return pcm


# ----------------------------
# SAVE SND
# ----------------------------
def save_snd(path, pcm_data, sample_rate):
    with open(path, "wb") as f:
        f.write(struct.pack("<II", sample_rate, len(pcm_data)))
        f.write(pcm_data.tobytes())


# ----------------------------
# LOAD SND
# ----------------------------
def load_snd(path):
    with open(path, "rb") as f:
        header = f.read(8)
        sample_rate, length = struct.unpack("<II", header)
        samples = np.frombuffer(f.read(), dtype=np.int16)

    return sample_rate, samples


# ----------------------------
# PLAY AUDIO
# ----------------------------
def play_audio(sample_rate, samples):
    import threading

    audio = samples.astype(np.float32) / 32767.0

    def play():
        sd.play(audio, samplerate=sample_rate)
        sd.wait()

    t = threading.Thread(target=play)
    t.start()

    try:
        input("Playing... press ENTER to stop\n")
        sd.stop()
    except KeyboardInterrupt:
        sd.stop()

    t.join()


# ----------------------------
# MAIN
# ----------------------------
def main():
    parser = argparse.ArgumentParser(description="Audio tool")

    parser.add_argument("input", help="Input file (.wav or .snd)")
    parser.add_argument("-o", "--output", help="Output SND file")
    parser.add_argument("-r", "--rate", type=int, default=DEFAULT_SAMPLE_RATE)
    parser.add_argument("--no-normalize", action="store_true")
    parser.add_argument("--play", action="store_true", help="Play input instead of converting")

    args = parser.parse_args()

    ext = os.path.splitext(args.input)[1].lower()

    # ----------------------------
    # PLAY MODE (SND only)
    # ----------------------------
    if args.play:
        if ext != ".snd":
            print("Error: --play expects a .snd file")
            return

        sr, samples = load_snd(args.input)
        play_audio(sr, samples)
        return

    # ----------------------------
    # CONVERT MODE (WAV → SND)
    # ----------------------------
    if ext != ".wav":
        print("Error: input must be a .wav file for conversion")
        return

    output_path = args.output or os.path.splitext(args.input)[0] + ".snd"

    pcm = convert_wav(
        args.input,
        target_sr=args.rate,
        normalize=not args.no_normalize
    )

    save_snd(output_path, pcm, args.rate)
    print(f"Saved: {output_path}")


if __name__ == "__main__":
    main()