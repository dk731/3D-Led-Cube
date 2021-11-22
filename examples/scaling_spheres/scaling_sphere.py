import time
import numpy as np


class ScalingSphere:
    R_DIFF_THRESH = 0.3
    EPSILON = 0.001

    def __init__(
        self,
        drawer,
        color,
        pos,
        start_r,
        scale_speed,
        layer_time,
    ):
        self.drawer = drawer
        self.spheres_list = []

        self.color = color
        self.pos = pos

        self.start_r = start_r
        self.scale_speed = scale_speed

        self.layer_time = layer_time
        self.animation_time = 0
        self.prev_time = None

    def update(self):
        t = time.time()
        if not self.prev_time:
            self.prev_time = t

        self.animation_time += t - self.prev_time
        self.prev_time = t

        self.spheres_list.clear()

        r = self.scale_speed * self.animation_time
        fade_start = self.animation_time - self.layer_time
        if fade_start < 0:
            fade_start = 0

        for t in np.arange(
            start=fade_start,
            stop=self.animation_time,
            step=self.R_DIFF_THRESH / self.scale_speed,
        ):
            fade_prog = (t - fade_start) / self.layer_time + self.EPSILON
            self.spheres_list.append(
                (
                    t * self.scale_speed + self.start_r,
                    [
                        max(0.0, min(c * fade_prog, 1.0)) for c in self.color
                    ],  # Clamp all color value to [0, 1] range
                )
            )

    def draw(self):
        for r, color in self.spheres_list:
            self.drawer.set_color(color)
            self.drawer.sphere(self.pos, r, 0.5)
