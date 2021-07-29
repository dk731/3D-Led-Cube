from ledcd import CubeDrawer
from time import sleep
import itertools
import random
import math
import colorsys
import threading

cd = CubeDrawer.get_obj()
cd.set_brigthness(1.0)

cur_h = 0.0
speed = 0.001
count = 0


def hsv2rgb(h, s, v):
    return tuple(round(i * 255) for i in colorsys.hsv_to_rgb(h, s, v))


def print_fps():
    while True:
        print("FPS: ", 1 / (cd.delta_time if cd.delta_time else 1))
        sleep(1)


cd.set_color(255, 0, 0)

threading.Thread(target=print_fps).start()

while True:
    cur_h += speed * cd.delta_time
    if cur_h < 0 or cur_h > 1:
        speed *= -1
    if cur_h < 0:
        cur_h = 0
    elif cur_h > 1:
        cur_h = 1

    color = hsv2rgb(cur_h, 1.0, 1.0)
    cd.clear(color)
    # cd.line()))
    cd.show()
    # input()
# while True:
#     cd.clear()
#     cd.rotate((0.01, 0.01, 0.01))
#     cd.circle((0, 0, 0), 5)
#     cd.draw()
#     sleep(0.01)
