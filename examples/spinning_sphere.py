from ledcd import CubeDrawer
import time
import threading
import math

cd = CubeDrawer.get_obj()
cd.translate(7.5, 7.5, 7.5)
cd.set_brigthness(0.0)

fps = 0


def fps_printer():
    global fps

    while True:
        print(fps)
        fps = 0
        time.sleep(1)


threading.Thread(target=fps_printer, daemon=True).start()

off_deg = 0


while True:
    cd.translate(math.sin(off_deg) / 50, 0, math.cos(off_deg) / 50)
    fps += 1
    # cd.clear(*[int(c * 255 * 0.1) for c in colorsys.hsv_to_rgb(deg, 1, 1)])
    cd.clear()

    # cd.set_color(*[int(c * 255) for c in colorsys.hsv_to_rgb(deg + 0.33, 1, 1)])
    cd.set_color(255)
    cd.sphere(0, 0, 0, 10, 8, 6, 1)

    # cd.set_color(*[int(c * 255) for c in colorsys.hsv_to_rgb(deg + 0.66, 1, 1)])
    cd.set_color(255, 0, 0)
    cd.sphere(0, 0, 0, 5, 4, 3, 1)

    cd.rotate(3.14 * cd.delta_time, 3.14 * cd.delta_time / 2, 0)
    off_deg += cd.delta_time / 2
    cd.show()
