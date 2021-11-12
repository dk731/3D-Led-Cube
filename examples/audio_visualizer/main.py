from ledcd import CubeDrawer as cd

from scipy.fft import rfft, rfftfreq
from scipy.io import wavfile

import numpy as np
import time

from playsound import playsound
import threading
from offset_sphere import OffsetSphere


def smooth_fourie(arr):
    return 1


drawer = cd.get_obj()
drawer.translate(7.5, 7.5, 7.5)
sp = OffsetSphere(drawer)

file_name = (
    "C://Users//user//Desktop//3D-Led-Cube//examples//audio_visualizer//song.wav"
)

rate, data = wavfile.read(file_name)


# If single channeled copy it and make 2 equal channels
if len(data.shape) != 2:
    (shape_size,) = data.shape
    data = np.concatenate([data, data], axis=None).reshape((shape_size, 2))


start_frame = 0
frame_size = rate // 2


# xf = np.log(rfftfreq(frame_size, 1 / rate) + 25)


threading.Thread(target=lambda: playsound(file_name), daemon=True).start()

start_time = time.time()
while True:
    yfl = (
        np.clip(
            np.abs(rfft(data[start_frame : start_frame + frame_size, 0])), 0.1, 35000000
        )
        / 35000000
    )
    yfr = (
        np.clip(
            np.abs(rfft(data[start_frame : start_frame + frame_size, 1])), 0.1, 35000000
        )
        / 35000000
    )

    drawer.clear()
    drawer.rotate(drawer.delta_time / 4, drawer.delta_time / 8, 0)
    sp.update_points(1.5, 2.5, yfl * 12, yfr * 12, 1.5)
    sp.draw()

    drawer.show()

    start_frame = int((time.time() - start_time) * rate)
