from ledcd import CubeDrawer
from math import sin, pi

drawer = CubeDrawer.get_obj()

# drawer.draw_immediate = True

t = 0

while True:
    drawer.clear()

    drawer.pop_matrix()

    drawer.translate(7.5, 7.5, 7.5)
    drawer.rotate(sin(t), sin(t / 2), 0)

    drawer.sphere(0, 0, 0, 10, 7, 7, 2)
    t += drawer.delta_time / 10

    drawer.show()

input()
