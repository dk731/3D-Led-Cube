# scipy, simpleaudio, numpy
# Working only on Windows!
from ledcd import CubeDrawer as cd

from scipy.fft import rfft, rfftfreq
from scipy.io import wavfile

import numpy as np
import time

import simpleaudio as sa
from offset_sphere import OffsetSphere


def smooth_fourie(arr):
    return 1


drawer = cd.get_obj()
drawer.translate(7.5, 7.5, 7.5)
drawer.set_fps_cap(0)
sp = OffsetSphere(drawer, 3)

file_path = "ENTER HERE PATH TO THE WAV FILE"

if file_path == "ENTER HERE PATH TO THE WAV FILE":
    print("Please provide some wav file")
    exit(0)

rate, data = wavfile.read(file_path)


# If single channeled copy it and make 2 equal channels
if len(data.shape) != 2:
    (shape_size,) = data.shape
    data = np.concatenate([data, data], axis=None).reshape((shape_size, 2))

start_frame = 0
frame_size = rate // 15

smooth_window = 30
norm_vec = np.exp(
    np.arange(-1, stop=0, step=1 / ((frame_size + 3 - smooth_window * 2) / 2)) * 2
)

wave_obj = sa.WaveObject.from_wave_file(file_path)
play_obj = wave_obj.play()
start_time = time.time()


while True:
    start_frame = int((time.time() - start_time) * rate)
    yfl = np.abs(rfft(data[start_frame : start_frame + frame_size, 0]))
    yfr = np.abs(rfft(data[start_frame : start_frame + frame_size, 1]))

    cumsum_vecl = np.cumsum(np.insert(yfl, 0, 0))
    cumsum_vecr = np.cumsum(np.insert(yfr, 0, 0))

    yfl = (cumsum_vecl[smooth_window:] - cumsum_vecl[:-smooth_window]) / smooth_window
    yfr = (cumsum_vecr[smooth_window:] - cumsum_vecr[:-smooth_window]) / smooth_window

    yfl *= norm_vec
    yfr *= norm_vec

    yfl /= np.max(yfl)
    yfr /= np.max(yfr)

    drawer.clear()
    drawer.rotate(drawer.delta_time * 2, drawer.delta_time, drawer.delta_time / 4)
    sp.update_points(1.5, 8, 6, yfl, yfr, 2)
    sp.draw()

    drawer.show()
