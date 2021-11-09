from ledcd import CubeDrawer
import time
from math import sin
import colorsys

cd = CubeDrawer.get_obj()
cd.set_fps_cap(0)
cd.set_brigthness(0.1)
deg = 0
speed = 0.15
while True:
    cd.clear(*[c * 0.1 for c in colorsys.hsv_to_rgb(deg, 1, 1)])
    cd.show()
    deg += speed * cd.delta_time
    if deg > 1:
        deg = 0
