from PIL import Image
import numpy as np
import ujson
import colorsys as cs

OFILE = "./5vsk_logo.json"
OUTPUT_SIZE = (16, 16)

MIN_CONTRAST = 0.9

image = Image.open(r"C:\Users\user\Downloads\object1495853628.png")
image = image.resize(OUTPUT_SIZE, Image.BILINEAR)

image.show()

np_arr = np.asanyarray(image)

# {position: {x: number, y: number}, color: RGB}
output_arr = []

print("Image shape: ", np_arr.shape)
POINTS_DISTANCE = max(1 / np_arr.shape[0], 1 / np_arr.shape[1])

for y, row in enumerate(np_arr):
    for x, pixel in enumerate(row):
        new_pixel = {
            "position": {
                "x": float(x * POINTS_DISTANCE),
                "y": float(y * POINTS_DISTANCE),
            },
            "color": [int(val) for val in np.flip(pixel[1:])],
        }
        if (
            cs.rgb_to_hls(*[val / 255.0 for val in new_pixel["color"]])[2]
            > MIN_CONTRAST
        ):
            output_arr.append(new_pixel)


with open(OFILE, "w+") as f:
    ujson.dump(output_arr, f)

print("Output written to: ", OFILE)
