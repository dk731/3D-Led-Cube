from CubeDrawer import *
from time import sleep
import itertools
import random
import math

red = (193, 0, 0)
white = (255, 255, 255)
cd = CubeDrawer(brigthness=0.05, sync=True)
cd.push_matrix()
cd.translate((7.5, 7, 3.5))


ang = 0
speed = 0.001
wavy = 2
wave_size = 3

while True:
    cd.clear()
    # cd.rotate((0, 0.01, 0.02))

    for i in range(10):
        if i < 6 and i > 3:
            cd.set_color(white)
        else:
            cd.set_color(red)
        zval = math.sin((i / 9 * math.pi * wavy) + ang)
        cd.line((-10, (i - 4), zval * wave_size), (10, (i - 4), zval * wave_size))
    # ang += speed
    cd.show()
