from ledcd import CubeDrawer
from time import sleep
import itertools
import random
import math
import colorsys

cd = CubeDrawer(0.05)

cur_h = 0.0
speed = 0.001
cd.translate(7, 7, 3)
zeros = (0, 0, 0)
while True:
    cd.clear()

    cd.set_color(255, 255, 255)

    cd.line((-10, -10, -10), zeros)
    cd.line((10, 10, 10), zeros)
    cd.line((10, -10, -10), zeros)
    cd.line((-10, 10, -10), zeros)
    cd.line((-10, -10, 10), zeros)
    cd.line((10, 10, -10), zeros)
    cd.line((-10, 10, 10), zeros)
    cd.line((10, -10, 10), zeros)

    cd.set_color(255, 0, 0)
    cd.set_pixel(0, 0, 0)

    input()

    cd.show()
# while True:
#     cd.clear()
#     cd.rotate((0.01, 0.01, 0.01))
#     cd.circle((0, 0, 0), 5)
#     cd.draw()
#     sleep(0.01)
