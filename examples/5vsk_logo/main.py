# ujson
import ujson
from ledcd import CubeDrawer
from math import tau

cd = CubeDrawer.get_obj()

IMG_SIZE = 16
PIXEL_SIZE = IMG_SIZE / 20

# Load image
with open(
    r"C:\Users\user\Documents\3D-Led-Cube\examples\5vsk_logo\5vsk_logo.json", "r"
) as f:
    img = ujson.load(f)
cd.translate(7.5, 7.5, 7.5)
cd.set_color(255)

while True:
    cd.clear()
    cd.rotate(tau * cd.delta_time / 6, tau * cd.delta_time / 24, 0)

    for pixel in img:
        pixel_position = (
            pixel["position"]["x"] * IMG_SIZE - IMG_SIZE / 2,
            pixel["position"]["y"] * IMG_SIZE - IMG_SIZE / 2,
            0,
        )
        cd.filled_sphere(pixel_position, PIXEL_SIZE)

    cd.show()
