from distutils.core import setup, Extension

extra_link = [
    "/usr/local/lib64/libws.a",
    "-lpthread",
    "-lrt",
    "-lblas",
    "-lGLEW",
    "-lGL",
    "-lglfw",
    "-L/usr/lib/x86_64-linux-gnu/",
]

extra_macros = [("VIRT_CUBE", None)]
led_module = Extension(
    "_ledcd",
    sources=["swig_module_wrap.cxx", "CubeDrawer.cpp"],
    extra_link_args=extra_link,
    define_macros=extra_macros,
    # extra_objects=["/usr/local/lib64/libws.a"]
    # library_dirs=["/usr/local/lib64/"],
    # libraries=["libws.a"]
)

setup(
    name="LedCD",
    version="0.1",
    author="LedCube",
    description="""Led Cube driver module""",
    ext_modules=[led_module],
    py_modules=["ledcd"],
)
