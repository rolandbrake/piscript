# from PIL import Image

# IMAGE_PATH = "pi.png"
# W, H = 20, 20
# YELLOW_VAL = 9
# BLACK_VAL = 0

# # Load and convert to grayscale
# img = Image.open(IMAGE_PATH).convert("L")

# # --- Auto threshold using image mean ---
# pixels = list(img.getdata())
# threshold = sum(pixels) / len(pixels)

# # Binarize FIRST
# bw = img.point(lambda p: 255 if p > threshold else 0, mode="1")

# # Resize AFTER binarization
# bw = bw.resize((W, H), Image.NEAREST)

# pixels = bw.load()

# print(f"uint8_t site_qr[{H}][{W}] = {{")

# for y in range(H):
#     row = []
#     for x in range(W):
#         row.append(str(YELLOW_VAL if pixels[x, y] else BLACK_VAL))
#     print("  {" + ",".join(row) + "},")
    
# print("};")
from PIL import Image
import numpy as np

# -------- CONFIG --------
IMAGE_PATH = "repl.png"   # path to your image
WIDTH  = 100
HEIGHT = 12

# palette: value -> RGB
PALETTE = {
    0:  np.array([255, 255, 255]),  # white
    5:  np.array([0,   0,   0]),    # black
    8:  np.array([255, 0,   0]),    # red
    12: np.array([0,   0, 255]),    # blue
}
# ------------------------

def nearest_color(pixel):
    return min(
        PALETTE.keys(),
        key=lambda k: np.linalg.norm(pixel - PALETTE[k])
    )

# load + resize
img = Image.open(IMAGE_PATH).convert("RGB")
img = img.resize((WIDTH, HEIGHT), Image.NEAREST)

pixels = np.array(img)

# convert to indexed values
data = []
for y in range(HEIGHT):
    row = []
    for x in range(WIDTH):
        row.append(nearest_color(pixels[y, x]))
    data.append(row)

# output as C array
print(f"int image[{HEIGHT}][{WIDTH}] = {{")
for row in data:
    print("  { " + ", ".join(f"{v:2d}" for v in row) + " },")
print("};")

