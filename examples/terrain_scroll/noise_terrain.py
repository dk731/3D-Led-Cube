import colorsys
from random import random
from opensimplex import OpenSimplex
import numpy as np
import time


class NoiseTerrain:
    def __init__(self, drawer, grid_size, cell_size, noise_scale=1, noise_amp=1):
        self.noise_obj = OpenSimplex(seed=int(time.time() * 1000))

        self.drawer = drawer

        self.grid_size = grid_size
        self.cell_size = cell_size
        self.half_size = cell_size / 2

        self.noise_scale = noise_scale
        self.noise_amp = noise_amp

        self.heights = []

    def update(self, start_x=0, start_z=0):
        self.heights.clear()
        for z in range(self.grid_size[1] + 1):
            for x in range(self.grid_size[0] + 1):
                self.heights.append(
                    (
                        self.noise_obj.noise2d(
                            x=(start_x + x) * self.noise_scale,
                            y=(start_z + z) * self.noise_scale,
                        )
                        + 1
                    )
                    * -self.noise_amp
                )

    def draw(self):
        for z in range(self.grid_size[1]):
            for x in range(self.grid_size[0]):

                start_x = x - self.half_size
                start_z = z - self.half_size

                height_row1 = x + z * (self.grid_size[0] + 1)
                height_row2 = x + (z + 1) * (self.grid_size[0] + 1)

                self.drawer.poly(
                    (start_x, self.heights[height_row1], start_z),
                    (start_x + self.cell_size, self.heights[height_row1 + 1], start_z),
                    (
                        start_x + self.cell_size,
                        self.heights[height_row2],
                        start_z + self.cell_size,
                    ),
                    2,
                )
                self.drawer.poly(
                    (start_x, self.heights[height_row1], start_z),
                    (start_x, self.heights[height_row2 + 1], start_z + self.cell_size),
                    (
                        start_x + self.cell_size,
                        self.heights[height_row2],
                        start_z + self.cell_size,
                    ),
                    2,
                )


class TreeObj:
    def __init__(self, drawer, pos):
        self.drawer = drawer
        self.pos = pos
        self.start_time = time.time()

        self.wood_height = random() * 3 + 4
        self.leaves_size = (1.8, self.wood_height / 1.8, 1.8)

        self.leaves_color = colorsys.hsv_to_rgb(
            random() * 0.125 + 0.25, 1, 1 - (random() * 0.25 + 0.3)
        )

        self.wood_color = colorsys.hsv_to_rgb(
            0.0972 + random() * 0.0139, 1, 1 - (random() * 0.25 + 0.3)
        )

    def draw(self, offset_x, offset_z):
        self.drawer.set_color(self.wood_color)
        self.drawer.line(
            (self.pos[0] + offset_x, self.pos[1], self.pos[2] + offset_z),
            (
                self.pos[0] + offset_x,
                self.pos[1] - self.wood_height,
                self.pos[2] + offset_z,
            ),
            0.8,
        )

        self.drawer.set_color(self.leaves_color)
        self.drawer.filled_ellipsoid(
            (
                self.pos[0] + offset_x,
                self.pos[1] - self.wood_height,
                self.pos[2] + offset_z,
            ),
            self.leaves_size,
        )
