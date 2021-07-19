from distutils.core import setup, Extension

extra_flags = ["-lrt", "-lopenblas"]
# extra_libraries = ["/home/pi/OpenBlas/"]
led_module = Extension(
    "_ledcd",
    sources=["swig_module_wrap.cxx", "CubeDrawer.cpp"],
    extra_link_args=extra_flags,
)

setup(
    name="LedCD",
    version="0.1",
    author="LedCube",
    description="""Led Cube driver module""",
    ext_modules=[led_module],
    py_modules=["ledcd"],
)
