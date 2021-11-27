from ledcd import CubeDrawer
from falling_helper import *
import colorsys
from random import random
import threading

drawer = CubeDrawer.get_obj()

plane = FallingWall(0, 1.5, colorsys.hsv_to_rgb(random(), 1, 1), drawer)

# drawer.translate(7.5, 7.5, 7.5)
# drawer.scale(0.5, 0.5, 0.5)


def plane_updater():
    global plane
    counter = 0
    while True:
        time.sleep(4.5)
        counter += 1
        plane = FallingWall(counter, 1.5, colorsys.hsv_to_rgb(random(), 1, 1), drawer)


threading.Thread(target=plane_updater, daemon=True).start()

while True:
    drawer.clear()

    plane.update()
    plane.draw()

    drawer.show()
