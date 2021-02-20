from multiprocessing import shared_memory, Semaphore
from math import floor, pi, sin, cos
import ast
import warnings


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
        self.colors = bytearray(16 ** 3)
        self.current_color = (255, 255, 255)

        self.shm_obj.buf[-1] = 0b00000100 if self._is_sync else 0b00000000

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
        pp = [a for a in p]
        for tran in self.transform_list:
            xx, yy, zz = pp
            rx, ry, rz = tran[1]
            pp[0] = (
                (cos(rx) * cos(ry) * cos(rz) - sin(rx) * sin(rz)) * xx
                + (sin(rx) * cos(ry) * cos(rz) + cos(rx) * sin(rz)) * yy
                + (-sin(ry) * cos(rz)) * zz
            )

            pp[1] = (
                (-cos(rx) * cos(ry) * sin(rz) - sin(rx) * cos(rz)) * xx
                + (-sin(rx) * cos(ry) * sin(rz) + cos(rx) * cos(rz)) * yy
                + (sin(ry) * sin(rz)) * zz
            )

            pp[2] = (cos(rx) * sin(ry)) * xx + (sin(rx) * sin(ry)) * yy + (cos(ry)) * zz

            pp[0] += tran[0][0]
            pp[1] += tran[0][1]
            pp[2] += tran[0][2]

        pp = (floor(pp[0] + 0.5), floor(pp[1] + 0.5), floor(pp[2] + 0.5))

        for a in pp:
            if a not in range(16):
                return

        cur_p = (self.size[2] * self.size[1] * pp[2] + self.size[0] * pp[1] + pp[0]) * 3
        self.colors[cur_p] = self.current_color[0]
        self.colors[cur_p + 1] = self.current_color[1]
        self.colors[cur_p + 2] = self.current_color[2]

    def push_matrix(self):
        self.transform_list.append([[0, 0, 0], [0, 0, 0], [1, 1, 1]])

    def translate(self, p):
        self.transform_list[-1][0][0] += p[0]
        self.transform_list[-1][0][1] += p[1]
        self.transform_list[-1][0][2] += p[2]

    def rotate(self, p):  # Radians
        self.transform_list[-1][1][0] += p[0]
        self.transform_list[-1][1][1] += p[1]
        self.transform_list[-1][1][2] += p[2]

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

    def __del__(self):
        self.shm_obj.close()