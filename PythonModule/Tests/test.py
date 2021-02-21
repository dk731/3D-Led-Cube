from CubeDrawer import *
from time import sleep
import itertools
import random
import math

red = (193, 0, 0)
white = (255, 255, 255)
cd = CubeDrawer(brigthness=0.5, sync=True)
cd.push_matrix()
cd.translate((7, 7, 3.5))

real_w = 12
real_h = real_w / 2
real_z = 3
real_stripw = real_h / 2.5

speed = 0.005
# speed = 0
offset = 0

resolution = 1.5

visible_size = math.tau

min_step = 0.15

while True:
    cd.clear()
    cd.rotate((0, 0.01, 0.02))

    cur_y = -real_h
    cur_z = 0
    count = 0
    while cur_y <= real_h:
        count += 1
        cd.set_color(white if abs(cur_y) < real_stripw else red)
        z_val = (
            math.sin((cur_y + real_h) / (real_h * 2) * visible_size + offset) * real_z
        )
        cd.line((-real_w, cur_y, z_val), (real_w, cur_y, z_val))
        cur_y += max(z_val / real_z, min_step) * resolution
        offset += speed
    # print(count)
    cd.show()
# while True:
#     cd.clear()
#     cd.rotate((0.01, 0.01, 0.01))
#     cd.circle((0, 0, 0), 5)
#     cd.draw()
#     sleep(0.01)
