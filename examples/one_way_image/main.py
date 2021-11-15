from ledcd import CubeDrawer as cd
import numpy as np
import cv2

drawer = cd.get_obj()

image = cv2.imread(
    r"C:\Users\user\Desktop\3D-Led-Cube\examples\one_way_image\mario.png"
)

resized = res = cv2.resize(image, dsize=(16, 16), interpolation=cv2.INTER_CUBIC)
drawer.translate(7.5, 7.5, 7)

while True:
    drawer.clear()

    for row in resized:
        for col in row:
            drawer.sphere()

    drawer.show()
