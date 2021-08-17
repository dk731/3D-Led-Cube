from distutils.core import setup, Extension
import os
import platform
import sys
import glob


extra_includes = ["./include"]
extra_library_dirs = ["./lib"]
runtime_libs = []

if platform.system() == "Windows":
    sys.path.append(os.path.join(os.getcwd(), "lib"))
    extra_link = glob.glob("./lib/*.a") + glob.glob("./lib/*.lib")
else:
    extra_link = ["-lws", "-lpthread", "-lGLEW", "-lglfw"]
    runtime_libs.append("./lib")

extra_macros = [("VIRT_CUBE", None), ("DYNAMIC_SHADER_INCLUDE", None)]
led_module = Extension(
    "_ledcd",
    sources=["src/swig_module_wrap.cxx", "src/CubeDrawer.cpp"],
    extra_link_args=extra_link,
    define_macros=extra_macros,
    include_dirs=extra_includes,
    library_dirs=extra_library_dirs,
    runtime_library_dirs=runtime_libs,
)

setup(
    name="LedCD",
    version="0.1",
    author="LedCube",
    description="""Led Cube driver module""",
    ext_modules=[led_module],
    py_modules=["ledcd"],
    package_dir={"": "src"},
)
