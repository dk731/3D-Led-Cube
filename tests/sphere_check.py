from ledcd import CubeDrawer
from math import sin, pi
import threading
from time import sleep

drawer = CubeDrawer.get_obj()

a = 0

def fps_printer():
    global a
    while True:
        print(a)
        a = 0
        sleep(1)

threading.Thread(target=fps_printer).start()


# drawer.draw_immediate = True

t = 0

while True:
    drawer.clear()

    drawer.pop_matrix()

    drawer.translate(7.5, 7.5, 7.5)
    drawer.rotate(sin(t), sin(t / 2), 0)

    drawer.sphere(0, 0, 0, 10, 7, 7, 2)
    t += drawer.delta_time * 4

    drawer.show()
    a += 1

input()
