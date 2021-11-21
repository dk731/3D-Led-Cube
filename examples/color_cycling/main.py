from ledcd import CubeDrawer
import time
from math import sin
import colorsys

cd = CubeDrawer.get_obj()

deg = 0
speed = 0.15
while True:
    cd.clear(colorsys.hsv_to_rgb(deg, 1, 1))
    cd.show()
    deg += speed * cd.delta_time
    if deg > 1:
        deg = 0
