from math import sin, cos, radians, tau, pi, floor
from matplotlib.pyplot import draw
import numpy as np
import colorsys
from random import random


class OffsetSphere:
    EPSILON = 0.0000000000001

    def __init__(self, screen):
        self.spehres = []
        self.screen = screen
        self.global_offset = random()

    def __get_real_offset(self, arr, arr_len, prog):
        if not arr_len:
            return 0

        float_id = prog * arr_len

        curp_id = floor(float_id)
        if curp_id >= arr_len:
            curp_id = 0

        nextp_id = curp_id + 1
        if nextp_id >= arr_len:
            nextp_id = 0

        return arr[curp_id] + (arr[nextp_id] - arr[curp_id]) * (float_id - curp_id)

    def update_points(
        self, r=3, resolution=5, offsets_xy=[], offsets_xz=[], line_width=0.8
    ):
        self.r = r
        self.sphere_res = radians(resolution)  # Place point every 'resolution' degrees

        xy_off_len = len(offsets_xy)
        xz_off_len = len(offsets_xz)

        self.spehres.clear()

        for y_a in np.arange(0.0, pi + self.EPSILON, self.sphere_res):
            sin_y = sin(y_a)

            xy_offset = self.__get_real_offset(offsets_xy, xy_off_len, y_a / pi) * abs(
                cos(y_a)
            )
            pass
            for x_a in np.arange(
                0.0, tau + self.EPSILON, self.sphere_res / (sin_y + self.EPSILON)
            ):
                xz_offset = self.__get_real_offset(
                    offsets_xz, xz_off_len, x_a / tau
                ) * abs(sin(y_a))

                cur_r = abs(sin_y) * self.r

                x = sin(x_a) * (cur_r + xy_offset + xz_offset)
                y = (cos(y_a) * (self.r + xy_offset)) * -1
                z = cos(x_a) * (cur_r + xz_offset)

                self.spehres.append(((x, y, z), line_width))

    def draw(self, colored=True):
        self.screen.push_matrix()

        for spehre in self.spehres:
            pos, width = spehre
            if colored:
                dist = (pos[0] ** 2 + pos[1] ** 2 + pos[2] ** 2) ** 0.5
                self.screen.set_color(
                    *[
                        int(c * 255)
                        for c in colorsys.hsv_to_rgb(
                            self.global_offset + dist / 10, 1, 1
                        )
                    ]
                )

            self.screen.filled_sphere(pos, width)

        self.global_offset += self.screen.delta_time / 15
        self.screen.pop_matrix()


if __name__ == "__main__":
    from ledcd import CubeDrawer as cd

    drawer = cd.get_obj()

    sp = OffsetSphere(drawer)

    drawer.translate(7.5, 7.5, 7.5)

    sp.update_points(2.5, 3, [], np.ones(10) * 3)
    sp.draw()

    drawer.show()

    input()
