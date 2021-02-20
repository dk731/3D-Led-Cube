from multiprocessing import shared_memory
from math import floor, pi, sin, cos
import ast
import warnings
import numpy as np


class CubeDrawer:
    def __init__(self, brigthness=0.1, sync=True):
        warnings.simplefilter("ignore")
        self.size = (16, 16, 16)
        self._is_sync = sync
        self.brigthness = brigthness

        try:
            self.shm_obj = shared_memory.SharedMemory(
                name="VirtualCubeSHMemmory", create=False
            )
        except:
            raise Exception("Was not able to open shared memmory object.")
        print(
            "Successfuly connected to shared memmory object, with size: {}".format(
                self.shm_obj.size
            )
        )

        self._init_drawing_()

    def _init_drawing_(self):
        self.transform_list = list()
        self.current_color = (255, 255, 255)
        self.colors = bytearray(16 ** 3)

        self.shm_obj.buf[-1] = (
            0b00000100 if self._is_sync else 0b00000000
        )  # set sync flag

    def show(self):
        while self.shm_obj.buf[-1] & 0b00000010:
            pass

        self.shm_obj.buf[-1] &= ~0b00000001

        self.shm_obj.buf[
            0 : len(self.colors)
        ] = self.colors  # loading from python array to shared memmory

        self.shm_obj.buf[-1] |= 0b00000001

    def clear(self, color=None):
        if color:
            self.colors = bytearray(bytes(color)) * 4092
        else:
            self.colors = bytearray(self.size[0] * self.size[1] * self.size[2] * 3)

    def set_size(self, size):
        self.cursor_size = size

    def set_color(self, color):
        self.current_color = (
            round(color[1] * self.brigthness),
            round(color[0] * self.brigthness),
            round(color[2] * self.brigthness),
        )

    def pixel_at(self, p):
        pp = np.append(np.array(p, np.single), np.array([1]))
        for tran in self.transform_list:
            if tran["recalc"]:
                tran["model_m"] = tran["trans"] @ tran["rot"] @ tran["scale"]
                tran["recalc"] = False
            pp = tran["model_m"] @ pp

        pp = [round(pp[x]) for x in range(3)]

        for a in pp:
            if a not in range(16):
                return

        cur_p = ((pp[2] * 16 + pp[1]) * 16 + pp[0]) * 3

        self.colors[cur_p] = self.current_color[0]
        self.colors[cur_p + 1] = self.current_color[1]
        self.colors[cur_p + 2] = self.current_color[2]

    def push_matrix(self):
        self.transform_list.append(
            {
                "trans": np.identity(4),
                "rot": np.identity(4),
                "scale": np.identity(4),
                "model_m": np.identity(4),
                "cur_angle": [0 for _ in range(3)],
                "recalc": False,
            }
        )

    def translate(self, p):
        # self.transform_list[-1][0][0] += p[0]
        # self.transform_list[-1][0][1] += p[1]
        # self.transform_list[-1][0][2] += p[2]
        cur_mat = self.transform_list[-1]["trans"]
        cur_mat[0][3] += p[0]
        cur_mat[1][3] += p[1]
        cur_mat[2][3] += p[2]
        self.transform_list[-1]["recalc"] = True

    def rotate(self, p):  # Radians
        # self.transform_list[-1][1][0] += p[0]
        # self.transform_list[-1][1][1] += p[1]
        # self.transform_list[-1][1][2] += p[2]
        self.transform_list[-1]["cur_angle"] = [(x + y) for x, y in zip(p, self.transform_list[-1]["cur_angle"])]
        cur_agnle = self.transform_list[-1]["cur_angle"]

        cur_mat = self.transform_list[-1]["rot"]

        cur_mat[0][0] = np.cos(cur_agnle[2]) * np.cos(cur_agnle[1])
        cur_mat[0][1] = np.cos(cur_agnle[2]) * np.sin(cur_agnle[1]) * np.sin(
            cur_agnle[0]
        ) - np.sin(cur_agnle[2]) * np.cos(cur_agnle[0])
        cur_mat[0][2] = np.cos(cur_agnle[2]) * np.sin(cur_agnle[1]) * np.cos(
            cur_agnle[0]
        ) + np.sin(cur_agnle[2]) * np.sin(cur_agnle[0])

        cur_mat[1][0] = np.sin(cur_agnle[2]) * np.cos(cur_agnle[1])
        cur_mat[1][1] = np.sin(cur_agnle[2]) * np.sin(cur_agnle[1]) * np.sin(
            cur_agnle[0]
        ) + np.cos(cur_agnle[2]) * np.cos(cur_agnle[0])
        cur_mat[1][2] = np.sin(cur_agnle[2]) * np.sin(cur_agnle[1]) * np.cos(
            cur_agnle[0]
        ) + np.cos(cur_agnle[2]) * np.sin(cur_agnle[0])

        cur_mat[2][0] = -np.sin(cur_agnle[1])
        cur_mat[2][1] = np.cos(cur_agnle[1]) * np.sin(cur_agnle[0])
        cur_mat[2][2] = np.cos(cur_agnle[1]) * np.cos(cur_agnle[0])

        self.transform_list[-1]["recalc"] = True

    def scale(self, p):
        cur_mat = self.transform_list[-1]["scale"]

        cur_mat[0][0] *= p[0]
        cur_mat[1][1] *= p[1]
        cur_mat[2][2] *= p[2]

        self.transform_list[-1]["recalc"] = True

    def pop_matrix(self):
        if len(self.transform_list) > 0:
            self.transform_list.pop()

    def line(self, p1, p2):
        line = (p2[0] - p1[0], p2[1] - p1[1], p2[2] - p1[2])

        length = (line[0] ** 2 + line[1] ** 2 + line[2] ** 2) ** 0.5
        for i in range(floor(length + 0.5) + 1):
            t = i / length
            x = p1[0] + line[0] * t
            y = p1[1] + line[1] * t
            z = p1[2] + line[2] * t
            self.pixel_at([x, y, z])

    def box(self, p, size):
        p1 = []
        p2 = []
        for a, b in zip(p, size):
            p1.append(a - b / 2)
            p2.append(a + b / 2)

        self.line(p1, [p2[0], p1[0], p1[2]])
        self.line(p1, [p1[0], p2[0], p1[2]])
        self.line(p1, [p1[0], p1[0], p2[2]])

        self.line(p2, [p1[0], p2[0], p2[2]])
        self.line(p2, [p2[0], p1[0], p2[2]])
        self.line(p2, [p2[0], p2[0], p1[2]])

        self.line([p2[0], p1[0], p1[2]], [p1[0], p2[0], p2[2]])
        self.line([p2[0], p1[0], p1[2]], [p2[0], p1[0], p2[2]])

        self.line([p1[0], p2[0], p1[2]], [p1[0], p2[0], p2[2]])
        self.line([p1[0], p2[0], p1[2]], [p2[0], p1[0], p2[2]])

        self.line([p1[0], p1[0], p2[2]], [p1[0], p2[0], p2[2]])
        self.line([p1[0], p1[0], p2[2]], [p2[0], p1[0], p2[2]])

    def circle(self, p, radius):
        x = radius
        y = 0
        P = 1 - radius

        while x > y:
            y += 1
            if P <= 0:
                P = P + 2 * y + 1

            else:
                x -= 1
                P = P + 2 * y - 2 * x + 1

            if x < y:
                break

            self.pixel_at([x + p[0], y + p[1], p[2]])
            self.pixel_at([-x + p[0], y + p[1], p[2]])
            self.pixel_at([x + p[0], -y + p[1], p[2]])
            self.pixel_at([-x + p[0], -y + p[1], p[2]])

            if x != y:
                self.pixel_at([y + p[0], x + p[1], p[2]])
                self.pixel_at([-y + p[0], x + p[1], p[2]])
                self.pixel_at([y + p[0], -x + p[1], p[2]])
                self.pixel_at([-y + p[0], -x + p[1], p[2]])

        self.pixel_at([p[0] + radius, p[1], p[2]])
        self.pixel_at([p[0] - radius, p[1], p[2]])
        self.pixel_at([p[0], p[1] + radius, p[2]])
        self.pixel_at([p[0], p[1] - radius, p[2]])