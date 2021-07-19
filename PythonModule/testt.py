from ledcd import CubeDrawer
from time import sleep
import itertools
import random
import math
import colorsys

cd = CubeDrawer(0.05)

cur_h = 0.0
speed = 0.001


def hsv2rgb(h, s, v):
    return tuple(round(i * 255) for i in colorsys.hsv_to_rgb(h, s, v))


cd.set_color(255, 0, 0)

while True:
    cd.clear()
    cd.clear(hsv2rgb(cur_h, 1.0, 1.0))
    cur_h = cur_h + speed if cur_h < 1.0 else 0
    # cd.line()))
    cd.show()
    # input()
# while True:
#     cd.clear()
#     cd.rotate((0.01, 0.01, 0.01))
#     cd.circle((0, 0, 0), 5)
#     cd.draw()
#     sleep(0.01)
