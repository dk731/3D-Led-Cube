from distutils.core import setup, Extension
import os

print(os.listdir("."))
print()
print("Include: ", os.listdir("./include"))
print("Lib: ", os.listdir("./lib"))

extra_link = [
    "-lws",
    "-lcblas",
    "-lpthread",
    "-lGLEW",
    "-lGL",
    "-lglfw",
]


extra_includes = ["./include"]
extra_library_dirs = ["./lib"]

extra_macros = [("VIRT_CUBE", None), ("DYNAMIC_SHADER_INCLUDE", None)]
led_module = Extension(
    "_ledcd",
    sources=["src/swig_module_wrap.cxx", "src/CubeDrawer.cpp"],
    extra_link_args=extra_link,
    define_macros=extra_macros,
    include_dirs=extra_includes,
    library_dirs=extra_library_dirs,
    runtime_library_dirs=["./lib"],
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
