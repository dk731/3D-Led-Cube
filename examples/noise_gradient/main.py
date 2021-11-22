# opensimplex, numpy
from ledcd import CubeDrawer as cd
from opensimplex import OpenSimplex
import numpy as np
import time
import colorsys

GRID_RES = 1.5
VOXEL_SIZE = 3

noise = OpenSimplex(seed=int(time.time() * 1000))
noise_scale = 0.05
drawer = cd.get_obj()

anim_time = 0


while True:
    drawer.clear()

    for z in np.arange(start=0, stop=16, step=GRID_RES):
        for y in np.arange(start=0, stop=16, step=GRID_RES):
            for x in np.arange(start=0, stop=16, step=GRID_RES):
                noise_val = (
                    noise.noise4d(
                        x=x * noise_scale,
                        y=y * noise_scale,
                        z=z * noise_scale,
                        w=anim_time,
                    )
                    + 1
                )
                drawer.set_color(colorsys.hsv_to_rgb(noise_val, 1, 1))
                drawer.filled_sphere((x, y, z), VOXEL_SIZE)

    drawer.show()
    anim_time += drawer.delta_time / 6
