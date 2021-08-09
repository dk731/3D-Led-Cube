from ledcd import CubeDrawer
from time import sleep
import math

c = CubeDrawer.get_obj()
c.translate(7.5, 7.5, 7.5)
speed = (2 * math.pi) / 5
while True:
    c.clear()
    c.circle((0, 0, 0), (6, 6), 0.4, 0.51)
    c.rotate(speed * c.delta_time, speed * c.delta_time / 2, 0)
    c.push_matrix()
    c.rotate(0, math.pi / 2, 0)
    c.set_color(255, 0, 0)
    c.circle(0, 0, 0, 6, 6, 0.07, 3)
    c.pop_matrix()
    c.set_color(255)
    c.show()
