from ledcd import CubeDrawer as cd
from math import sin, tau

drawer = cd.get_obj()
drawer.translate(7.5, 7.5, 7.5)
time = 0

LINE_MIN_SIZE = 1
SIN_APLITUDE = 4
WAVE_MULT = 1
WAVE_SPEED = 4

while True:
    drawer.clear()
    # drawer.rotate(tau * drawer.delta_time / 6, tau * drawer.delta_time / 24, 0)
    for z in range(32):
        for x in range(32):
            zz = z - 15
            xx = x - 15
            center_offset = zz ** 2 + xx ** 2
            line_size = (
                sin(time * WAVE_SPEED - (center_offset / 15) * WAVE_MULT) + 1
            ) / 2 * SIN_APLITUDE + LINE_MIN_SIZE

            drawer.line((xx, line_size, zz), (xx, -line_size, zz), 1)

    drawer.show()
    time += drawer.delta_time
