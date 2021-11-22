from math import sin, cos, radians, tau, pi, floor
import numpy as np
import colorsys
from random import random


class OffsetSphere:
    EPSILON = 0.0000000000001
    HPI = pi / 2

    # fade - duration in frames count
    def __init__(self, screen, fade=1):
        self.fade_delay = fade
        self.frames_list = [[] for _ in range(fade)]
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
        self,
        r=3,
        offset_max_r=8,
        resolution=5,
        offsets_xy=[],
        offsets_xz=[],
        line_width=0.8,
    ):
        self.r = r
        self.sphere_res = radians(resolution)  # Place point every 'resolution' degrees

        xy_off_len = len(offsets_xy)
        xz_off_len = len(offsets_xz)

        self.frames_list.pop(0)
        cur_frame = []

        for y_a in np.arange(0.0, pi + self.EPSILON, self.sphere_res):
            sin_y = sin(y_a)

            if y_a < self.HPI:
                cur_offset = (
                    self.__get_real_offset(offsets_xy, xy_off_len, y_a / self.HPI)
                    * abs(cos(y_a))
                    * offset_max_r
                )
            else:
                cur_offset = (
                    self.__get_real_offset(
                        offsets_xz, xz_off_len, (y_a - self.HPI) / self.HPI
                    )
                    * abs(cos(y_a))
                    * offset_max_r
                )

            for x_a in np.arange(
                0.0, tau + self.EPSILON, self.sphere_res / (sin_y + self.EPSILON)
            ):
                xz_offset = self.__get_real_offset(
                    offsets_xz, xz_off_len, x_a / tau
                ) * abs(sin(y_a))

                rr = self.r + cur_offset
                cur_r = abs(sin_y) * rr

                x = sin(x_a) * (cur_r + xz_offset)
                y = (cos(y_a) * rr) * -1
                z = cos(x_a) * (cur_r + xz_offset)

                dist = (x ** 2 + y ** 2 + z ** 2) ** 0.5 / offset_max_r

                cur_frame.append(
                    (
                        (x, y, z),
                        line_width,
                        colorsys.hsv_to_rgb(self.global_offset + dist, 1, 1),
                    )
                )

        self.frames_list.append(cur_frame)

    def draw(self, colored=True):
        self.screen.push_matrix()

        for i, frame in enumerate(self.frames_list):
            col_mult = (i + 1) / self.fade_delay
            for spehre in frame:
                pos, width, col = spehre
                if colored:
                    self.screen.set_color([c * col_mult for c in col])
                    self.screen.filled_sphere(pos, width)

        self.global_offset += self.screen.delta_time / 20
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
