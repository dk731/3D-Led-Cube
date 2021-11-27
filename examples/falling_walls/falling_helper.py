from math import pi, tau
import time
import numpy as np
from random import random

class FallingWall:
    PLANE_WIDTH = 1.5
    def __init__(self, side, fall_time, color, drawer):
        self.side = side
        match side:
            case 0:
                self.pos = (0, 15, 0)
            case 1:
                self.pos = (0, 15, 15)
            case 2:
                self.pos = (15, 15, 15)
            case _:
                self.pos = (15, 15, 0)

        self.start_rot = (0, pi / 2 * side, 0)

        self.fall_a = pi / fall_time ** 2
        self.start_time = -1

        self.anim_time = fall_time

        self.drawer = drawer

        self.fall_rot = (0, 0, 0)
        self.color = color
        self.scatter = None
    
    def update(self):
        if self.scatter:
            self.scatter.update()
            return

        if self.start_time < 0:
            self.start_time = time.time()
        t = time.time() - self.start_time
        if t > self.anim_time:
            self.scatter = ScatterSquare(15, 15, self.PLANE_WIDTH, self.color, self.drawer)
            t = self.anim_time

        
        self.fall_rot = ((self.fall_a * t ** 2) / 2, 0, 0)


    def draw(self):
        if self.scatter:
            self.drawer.push_matrix()
            self.drawer.translate(0, 15, 0)
            self.drawer.scale(15, -1, 15)
            self.scatter.draw()
            self.drawer.pop_matrix()
            return

        self.drawer.push_matrix()
        self.drawer.translate(self.pos)
        self.drawer.rotate(self.start_rot)

        self.drawer.push_matrix()
        self.drawer.rotate(self.fall_rot)
        
        self.drawer.set_color(self.color)

        self.drawer.poly((0, 0, 0), (0, -15, 0), (0, 0, 15), self.PLANE_WIDTH)
        self.drawer.poly((0, 0, 15), (0, -15, 0), (0, -15, 15), self.PLANE_WIDTH)

        self.drawer.pop_matrix()
        self.drawer.pop_matrix()


class ScatterPiece:
    def __init__(self, p1, p2, p3, triangle_width, off_speed, rot_speed, color, drawer):
        self.c_pos = tuple(sum(p) / 3 for p in zip(p1, p2, p3)) # find center of triangle
        self.triangle = tuple(tuple(pn - pc for pn, pc in zip(p, self.c_pos)) for p in [p1, p2, p3])
        self.drawer = drawer
        self.anim_pos = (0, 0, 0)
        self.anim_rot = (0, 0, 0)
        self.off_speed = off_speed
        self.rot_speed = rot_speed
        self.color = color
        self.start_anim = -1
        self.triangle_width = triangle_width

    def update(self):
        if self.start_anim < 0:
            self.start_anim = time.time()
        t = time.time() - self.start_anim

        self.anim_pos = tuple(pc + ps * t for pc, ps in zip(self.c_pos, self.off_speed))
        self.rot_speed = tuple(r * t for r in self.rot_speed)

    def draw(self):
        self.drawer.push_matrix()

        self.drawer.translate(self.c_pos)
        self.drawer.translate(self.anim_pos)
        self.drawer.rotate(self.anim_rot)

        self.drawer.set_color(self.color)
        self.drawer.poly(*self.triangle, self.triangle_width)

        self.drawer.pop_matrix()
        

class ScatterSquare:
    ROT_SPEED = 1
    OFF_SPEED = 6
    MIN_ROT_SPEED = 0.2
    MIN_OFF_SPEED = 10

    def __init__(self, x_res, z_res, square_width, color, drawer):
        self.triangles = []

        new_x = x_res - 1
        new_z = z_res - 1

        noise_amp_x = 1 / new_x / 3
        noise_amp_z = 1 / new_z / 3

        prev_offs = [(xx / new_x + self.rand() * noise_amp_x, 0, 0) for xx in range(x_res)]

        self.color = color
        for z in range(1, z_res):
            if z == z_res - 1:
                cur_offs = [[xx / new_x + self.rand() * noise_amp_x, 0, 0] for xx in range(x_res)]
            else:
                cur_offs = [[xx / new_x + self.rand() * noise_amp_x, 0, z / new_z + self.rand() * noise_amp_z] for xx in range(x_res)]
            cur_offs[0][0] = 0
            cur_offs[-1][0] = 1

            for x in range(x_res - 1):
                
                rand_vec = np.random.rand(3) * 2 - 1
                rand_vec[1] = abs(rand_vec[1])
                rand_vec[1] *= 4
                rand_vec /= np.sqrt(np.sum(rand_vec**2))

                self.triangles.append(
                    ScatterPiece(
                        cur_offs[x],
                        prev_offs[x], 
                        cur_offs[x + 1], 
                        square_width, 
                        tuple(rand_vec * (self.OFF_SPEED * random() + self.MIN_OFF_SPEED)), 
                        tuple(self.norm_rand_vec() * (self.ROT_SPEED * random() + self.MIN_ROT_SPEED)), 
                        color,
                        drawer
                    )
                )

                rand_vec = np.random.rand(3) * 2 - 1
                rand_vec[1] = abs(rand_vec[1])
                rand_vec[1] *= 4
                rand_vec /= np.sqrt(np.sum(rand_vec**2))

                self.triangles.append(
                    ScatterPiece(
                        cur_offs[x + 1], 
                        prev_offs[x], 
                        prev_offs[x + 1], 
                        square_width, 
                        tuple(rand_vec * (self.OFF_SPEED * random() + self.MIN_OFF_SPEED)), 
                        tuple(self.norm_rand_vec() * (self.ROT_SPEED * random() + self.MIN_ROT_SPEED)), 
                        color,
                        drawer
                    )
                )

            prev_offs = cur_offs

    def norm_rand_vec(self):
        v = np.random.rand(3) * 2 - 1
        return v / np.sqrt(np.sum(v**2))
    
    def rand(self):
        return random() * 2 - 1

    def update(self):
        for triangle in self.triangles:
            triangle.update()

    def draw(self):
        for triangle in self.triangles:
            triangle.draw()