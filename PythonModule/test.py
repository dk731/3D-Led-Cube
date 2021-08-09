from ledcd import CubeDrawer
from time import sleep
import math

c = CubeDrawer.get_obj()
while True:
    c.clear()
    c.translate(0.2 * c.delta_time, 0.2 * c.delta_time, 0.2 * c.delta_time)
    c.circle(0, 0, 0, 6, 6, 0.6, 1.0)
    c.push_matrix()
    c.rotate(0, math.pi / 2, 0)
    c.set_color(255, 0, 0)
    c.circle(0, 0, 0, 6, 6, 0.07, 2.0)
    c.pop_matrix()
    c.set_color(255)
    c.show()
