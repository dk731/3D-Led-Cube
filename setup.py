from distutils.core import setup, Extension
import os
import platform
import sys
import glob

root_dir = os.getcwd()
lib_dir = os.path.join(root_dir, "lib")

extra_includes = ["./include"]
extra_library_dirs = [lib_dir]
runtime_libs = []

if platform.system() == "Windows":
    vcpkg_dir = f"C:\\vcpkg\\installed\\{ 'x64' if '64' in platform.architecture()[0] else 'x86' }-windows"
    sys.path.append(lib_dir)
    # extra_link = glob.glob(lib_dir + "/*.a") + glob.glob(lib_dir + "/*.lib")
#     extra_link = [
#         f"/DEFAULTLIB:{lib_dir}\\glfw3dll.lib",
#         f"/DEFAULTLIB:{lib_dir}\\glew32d.lib",
#         f"/DEFAULTLIB:opengl32.lib",
#     ]
    extra_link = [ f'/DEFAULTLIB:"{}"' + file2 for file in glob.glob(lib_dir + "/*.lib")]
    extra_link.append("/DEFAULTLIB:opengl32.lib")
    
    extra_includes.append(os.path.join(vcpkg_dir, "include"))
    extra_library_dirs.append(os.path.join(vcpkg_dir, "lib"))

    print("PATH TO THE VCPKG: ", vcpkg_dir)
    print(glob.glob(os.path.join(os.path.join(vcpkg_dir, "include") + "/*")))

else:
    extra_link = ["-lpthread", "-lGLEW", "-lglfw"]
    runtime_libs.append(lib_dir)

# extra_link[0] = "/DEFAULTLIB:" + extra_link[0]
# extra_link.append("/VERBOSE")
# extra_link.append("-ws")
# extra_link.append("-libws")

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
