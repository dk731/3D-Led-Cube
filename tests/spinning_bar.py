from ledcd import CubeDrawer
import time
from math import sin

cd = CubeDrawer.get_obj()
cd.translate(7.5, 7.5, 7.5)

min_size = 3
scale_speed = 1.5
scale_scale = 4

t = 0

while True:
    cd.clear(0)
    scale_sin = min_size + (sin(t * scale_speed) + 1) / 2 * scale_scale
    cd.line(0, -scale_sin, 0, 0, scale_sin, 0, 1.5)
    cd.rotate(
        3.14 / 2 * cd.delta_time, 3.14 / 8 * cd.delta_time, 3.14 / 16 * cd.delta_time
    )

    cd.show()
    t += cd.delta_time
