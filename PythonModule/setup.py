from distutils.core import setup, Extension


led_module = Extension(
    "_ledcd",
    sources=["swig_module_wrap.cxx", "CubeDrawer.cpp"],
    extra_compile_args=["-lrt"],
)

setup(
    name="LedCD",
    version="0.1",
    author="LedCube",
    description="""Led Cube driver module""",
    ext_modules=[led_module],
    py_modules=["ledcd"],
)
