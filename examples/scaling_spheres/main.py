# numpy
from ledcd import CubeDrawer as cd
from scaling_sphere import ScalingSphere
import threading
import time
from random import random
import colorsys

drawer = cd.get_obj()

spheres_list = []

ANIM_CLEAR_TIME = 10


def random_spawner():
    global spheres_list
    DELTA_SLEEP = 0.8
    MIN_SLEEP = 1
    while True:
        spheres_list.append(
            ScalingSphere(
                drawer,
                colorsys.hsv_to_rgb(random(), 1, 1),
                [random() * 21 - 3 for _ in range(3)],
                1,
                random() * 4 + 8,
                random() * 0.5 + 0.3,
            )
        )
        time.sleep(random() * DELTA_SLEEP + MIN_SLEEP)


def list_cleaner():
    global spheres_list
    while True:
        while True:
            if spheres_list and spheres_list[0].animation_time > ANIM_CLEAR_TIME:
                spheres_list.pop(0)
            else:
                break
        time.sleep(1)


threading.Thread(target=random_spawner, daemon=True).start()
threading.Thread(target=list_cleaner, daemon=True).start()

while True:
    drawer.clear()

    for sphere in spheres_list:
        sphere.update()

        sphere.draw()

    drawer.show()
