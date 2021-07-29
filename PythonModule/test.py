from ledcd import CubeDrawer
from time import sleep
import math

red = (193, 0, 0)
white = (255, 255, 255)
cd = CubeDrawer.get_obj()

cd.translate((7.5, 7.5, 4.5))
cd.set_brigthness(1.0)
real_w = 12
real_h = real_w / 2
real_z = 3
real_stripw = real_h / 2.5

speed = 5
offset = 0

resolution = 1

visible_size = math.tau

min_step = 0.1

rotation_speed = 0.01

while True:
    cd.clear()
    cd.rotate((0, rotation_speed * cd.delta_time / 3, rotation_speed * cd.delta_time))

    cur_y = -real_h
    cur_z = 0
    count = 0

    while cur_y <= real_h:
        count += 1
        cd.set_color(white if abs(cur_y) < real_stripw else red)
        z_val = (
            math.sin((cur_y + real_h) / (real_h * 2) * visible_size + offset) * real_z
        )
        cd.line((-real_w, cur_y, z_val), (real_w, cur_y, z_val))
        cur_y += max(z_val / real_z, min_step) * resolution

    offset += speed * cd.delta_time

    cd.show()
    # print(round(1 / cd.delta_time), "Lines: ", count, "      ", end="\r")
