from CubeDrawer import *
from time import sleep
import itertools
import random
import math

cd = CubeDrawer(brigthness=0.05, sync=True)
cd.push_matrix()
cd.set_color((255, 255, 255))
cd.translate((7.5, 7.5, 2))
while True:
    cd.clear()
    cd.rotate((0, 0.001, 0.01))
    # cd.line((0, 0, 0), (15, 15, 0))
    cd.line((-7.5, -7.5, 0), (7.5, 7.5, 0))
    cd.show()
# while True:
#     cd.clear()
#     cd.rotate((0.01, 0.01, 0.01))
#     cd.circle((0, 0, 0), 5)
#     cd.draw()
#     sleep(0.01)
