from ledcd import CubeDrawer as cd
from noise_terrain import NoiseTerrain, TreeObj
from math import sin, cos
import time
import threading
from random import random

X_OFFSET_SPEED = 0.1
Z_OFFSET_SPEED = 0.13

drawer = cd.get_obj()
drawer.translate(0, 16, 0)
drawer.set_brigthness(0.5)

terrain = NoiseTerrain(drawer, (15, 15), 2, 0.1, 3)
trees_list = []

anim_time = 0
x_offset = 0
z_offset = 0


def tree_spawner():
    global trees_list
    while True:
        coords = (
            (x_offset + 17 + (random() * 2 - 1) * 7),
            (z_offset + 17 + +(random() * 2 - 1) * 7),
        )
        height = (
            terrain.noise_obj.noise2d(
                x=coords[0] * terrain.noise_scale,
                y=coords[1] * terrain.noise_scale,
            )
            + 1
        ) * -terrain.noise_amp
        trees_list.append(TreeObj(drawer, (coords[0], height, coords[1])))
        time.sleep((X_OFFSET_SPEED + Z_OFFSET_SPEED) / 2 * 5)


def tree_cleaner():
    global trees_list
    while True:
        while True:
            if trees_list and time.time() - trees_list[0].start_time > 20:
                trees_list.pop(0)
            else:
                break
        time.sleep(1)


threading.Thread(target=tree_spawner, daemon=True).start()
threading.Thread(target=tree_cleaner, daemon=True).start()

while True:
    drawer.clear()

    terrain.update(x_offset, z_offset)

    drawer.set_color(50)
    terrain.draw()

    for tree in trees_list:
        tree.draw(-x_offset, -z_offset)

    drawer.show()
    anim_time += drawer.delta_time
    x_offset += abs(sin(anim_time)) * X_OFFSET_SPEED
    z_offset += abs(cos(anim_time)) * Z_OFFSET_SPEED
