# ujson
import ujson
from ledcd import CubeDrawer
from math import tau
from random import random

cd = CubeDrawer.get_obj()

BOX_SIZE = 3
padding2 = 16 - BOX_SIZE * 2
box_position = [random() * padding2, random() * padding2, random() * padding2]

dx = random() * 2 + 0.5
dy = random() + 0.2
dz = random() * 1.5 + 0.35

d_arr = [dx, dy, dz]

cd.set_color(255)

while True:
    cd.clear()
    for 
    box_position[0] += dx * cd.delta_time
    box_position[1] += dy * cd.delta_time
    box_position[2] += dz * cd.delta_time

    cd.pop_all_matrix()
    cd.translate(box_position)

    cd.show()
