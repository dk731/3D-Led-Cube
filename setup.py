from distutils.core import setup, Extension
from distutils.command import build
import os
import platform
import sys
import glob
import subprocess
import requests
import shutil

root_dir = os.getcwd()

include_dirs = []


class CustomBuild(build.build):
    def __init__(self, *args):
        super().__init__(*args)

        self.root_dir = os.getcwd()
        self.lib_dir = os.path.join(self.root_dir, "lib")
        self.bin_dir = os.path.join(self.root_dir, "bin")
        self.tmp_dir = os.path.join(self.root_dir, "tmp")

        self.python_dll_dir = os.path.join(
            os.path.dirname(sys.executable), "Lib", "site-packages"
        )

        self.arch = 64 if sys.maxsize > 2 ** 32 else 32
        self.system = platform.system()

        self.include_dirs = []
        self.compiled_libs = []

        self.errors_count = 0

        # Create necessarily dirs
        if not os.path.exists(self.lib_dir):
            os.mkdir(self.lib_dir)
        if not os.path.exists(self.bin_dir):
            os.mkdir(self.bin_dir)
        if not os.path.exists(self.tmp_dir):
            os.mkdir(self.tmp_dir)

    def run(self):
        global led_module
        self.install_dependencies()
        led_module.include_dirs = self.include_dirs
        if self.system == "Windows":
            # lib_files = glob.glob(os.path.join(self.lib_dir, "*.*"))
            lib_files = [
                os.path.join(self.lib_dir, file)
                for file in ["glew32.lib", "glfw3dll.lib"]
            ]
            lib_files.append("opengl32.lib")

            led_module.extra_link_args = [f"/DEFAULTLIB:{lib}" for lib in lib_files]
        else:
            led_module.extra_link_args = ["-lpthread", "-lGLEW", "-lglfw"]
        build.build.run(self)

        for file in glob.glob(os.path.join(self.bin_dir, "*.*")):
            shutil.copy(file, self.python_dll_dir)

    def install_dependencies(self):
        print("Prepearing dependencies for build ...\n\n")

        print("  * Installing GLM ... [1/4]: ")
        self.install_glm()

        print("  * Installing WebSocket++ ... [2/4]: ")
        self.install_wsspp()

        print("  * Installing GLEW ... [3/4]: ")
        self.install_glew()

        print("  * Installing GLFW ... [4/4]: ")
        self.install_glfw()

        print(f"Finished installing dependencies with: {self.errors_count} errors ...")

    """
    * Download GLM header files
    """

    def install_glm(self):
        os.chdir(self.root_dir)
        print("    Downloading GLM from https://github.com/g-truc/glm ... ", end="")
        if self.call("git clone --depth 1 https://github.com/g-truc/glm") == 0:
            self.include_dirs.append(os.path.join(root_dir, "glm"))
            print("  Succesfully installed GLM headers !")
        else:
            print("  ### Was not able to install GLM headers !")

    """
    * Download WebSocket++ header files
    * Check if ASIO installed, if not:
        # On Windows:
            ~ Install using vcpkg
        # On Linux:
            ~ Install using package manager (apt/yum)
    """

    def install_wsspp(self):
        os.chdir(self.root_dir)
        # Download and setup headers
        print(
            "    Downloading WebSocket++ from https://github.com/zaphoyd/websocketpp ...",
            end="",
        )
        if self.call("git clone --depth 1 https://github.com/zaphoyd/websocketpp") == 0:
            self.include_dirs.append(os.path.join(root_dir, "websocketpp"))
            print("  Succesfully installed WebSocket++ headers !")
        else:
            print("  ### Was not able to install WebSocket++ headers !")

        # Check ASIO
        if self.system == "Windows":  # Try installing using vcpkg
            print("  Trying to install asio core 32/64 bit packages ... ", end="")
            self.call(
                "C:\\vcpkg\\vcpkg.exe install asio[core]:x64-windows asio[core]:x86-windows"
            )
            print("    Adding vcpkg includes and libs ...")

            potential_dirs = [
                "C:\\vcpkg\\installed\\x64-windows\\include",
                "C:\\vcpkg\\installed\\x86-windows\\include",
            ]

            for dir in potential_dirs:
                if os.path.isdir(dir):
                    print(f"      {dir} OK")
                    self.include_dirs.append(dir)
                else:
                    print(f"      {dir} ERROR")
                    self.errors_count += 1

        elif self.system == "Linux":  # Try installing using yum, apt ...
            print(
                "  Trying to install ASIO: (Ignore error if at least one finished with OK)"
            )
            results = []
            for c in [
                "sudo apt-get -y install asio-dev",
                "sudo yum install -y asio-devel",
                "sudo dnf --enablerepo=powertools install asio-devel",
                "sudo packman -Syu asio",
            ]:
                print(f"    Trying to install asio by running: {c} ... ", end="")
                results += self.call(c)

            if any([res == 0 for res in results]):
                print("  Succesfully installed ASIO !")
            else:
                self.errors_count += 1
                print("  ### Was not able to install ASIO !")

        elif self.system == "MacOS":  # Try installing using brew
            print("    Trying to install asio package using brew ... ", end="")
            if self.call("brew install asio") == 0:
                print("  Succesfully installed ASIO !")
            else:
                print("  ### Was not able to install ASIO !")

    """
    # On Windows:
        ~ Download prebuild binaries from github
    # On Linux:
        ~ Download source files and build
    """

    def install_glew(self):
        os.chdir(self.root_dir)

        # Windows build is very inconsistent,
        # to improve it was decided to use precompiled binaries
        tmp_glew = os.path.join(self.tmp_dir, "glew")
        if self.system == "Windows":
            print("    Downloading Windows release ... ", end="")
            if not self.download_release(
                "https://api.github.com/repos/nigels-com/glew/releases/latest",
                tmp_glew,
                "win32",
            ):
                print("  Was not able to download latest GLEW Windows release ... ")
                self.errors_count += 1
                return

            shutil.unpack_archive(tmp_glew, root_dir, format="zip")

            glew_dir = glob.glob("*glew*")[0]

            for file in glob.glob(  # Move all .lib's
                os.path.join(
                    glew_dir,
                    "lib",
                    "Release",
                    "x64" if self.arch == 64 else "Win32",
                    "*.lib",
                )
            ):
                shutil.move(file, self.lib_dir)

            for file in glob.glob(  # Move all .lib's
                os.path.join(
                    glew_dir,
                    "bin",
                    "Release",
                    "x64" if self.arch == 64 else "Win32",
                    "*.dll",
                )
            ):
                shutil.move(file, self.bin_dir)

        else:
            print("    Downloading Source files ... ", end="")
            if not self.download_release(
                "https://api.github.com/repos/nigels-com/glew/releases/latest",
                tmp_glew,
                "tgz",
            ):
                print("  Was not able to download latest GLEW source files ... ")
                self.errors_count += 1
                return
            shutil.unpack_archive(tmp_glew, root_dir, format="tar")

            glew_dir = glob.glob("*glew*")[0]

            os.mkdir(os.path.join(glew_dir, "build", "cmake", "build"))
            os.chdir(os.path.join(glew_dir, "build", "cmake", "build"))

            print("    Prepearing Makefile for build ... ", end="")
            self.call("cmake -DBUILD_SHARED_LIBS=ON ..")
            print("    Building GLEW ... ", end="")
            self.call("cmake --build .")
            # TODO: Move .lib and .so to lib and bin folders

            # for file in glob.glob("../../../include/GL/*"):
            #     shutil.move(file, os.path.join(include_dir, "GL"))
            # for file in glob.glob("./lib/Debug/*"):
            #     shutil.move(file, lib_dir)
            # for file in glob.glob("./bin/Debug/*"):
            #     shutil.move(file, lib_dir)

        self.include_dirs.append(os.path.join(glew_dir, "include"))

    """
    # On Windows:
        ~ Download prebuild binaries from github
    # On Linux:
        ~ Download source files and build
    """

    def install_glfw(self):
        os.chdir(self.root_dir)

        # Windows build is very inconsistent,
        # to improve it was decided to use precompiled binaries
        tmp_glfw = os.path.join(self.tmp_dir, "glfw")

        if self.system == "Windows":
            print("    Downloading Windows latest GLFW release ... ", end="")
            if not self.download_release(
                "https://api.github.com/repos/glfw/glfw/releases/latest",
                tmp_glfw,
                "WIN64" if self.arch == 64 else "WIN32",
            ):
                print("  Was not able to download latest GLFW Windows release ... ")
                self.errors_count += 1
                return

            shutil.unpack_archive(tmp_glfw, root_dir, format="zip")

            glfw_dir = glob.glob("*glfw*")[0]

            for file in glob.glob(  # Move all .lib's
                os.path.join(
                    glfw_dir,
                    "lib-vc2019",
                    "*.lib",
                )
            ):
                shutil.move(file, self.lib_dir)
            for file in glob.glob(  # Move all .lib's
                os.path.join(
                    glfw_dir,
                    "lib-vc2019",
                    "*.dll",
                )
            ):
                shutil.move(file, self.bin_dir)
        else:
            print("    Downloading Source files ... ", end="")
            if not self.download_release(
                "https://api.github.com/repos/nigels-com/glew/releases/latest",
                tmp_glfw,
                "tgz",
            ):
                print("  Was not able to download latest GLEW source files ... ")
                self.errors_count += 1
                return
            shutil.unpack_archive(tmp_glfw, root_dir, format="tar")

            glfw_dir = glob.glob("*glfw*")[0]

            os.mkdir(os.path.join(glfw_dir, "build", "cmake", "build"))
            os.chdir(os.path.join(glfw_dir, "build", "cmake", "build"))

            print("    Prepearing Makefile for build ... ", end="")
            self.call("cmake -DBUILD_SHARED_LIBS=ON ..")
            print("    Building GLEW ... ", end="")
            self.call("cmake --build .")

            # for file in glob.glob("../../../include/GL/*"):
            #     shutil.move(file, os.path.join(include_dir, "GL"))
            # for file in glob.glob("./lib/Debug/*"):
            #     shutil.move(file, lib_dir)
            # for file in glob.glob("./bin/Debug/*"):
            #     shutil.move(file, lib_dir)

        self.include_dirs.append(os.path.join(glfw_dir, "include"))

    # subprocess.call - without stdout

    def call(self, call_str: str):
        res = (
            0
            if subprocess.call(
                call_str,
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
                shell=True,
            )
            == 0
            else 1
        )
        self.errors_count += res
        print("ERROR" if res else "OK")
        return res

    def download_release(self, api_url, ofile, name_key_word):
        output_file = os.path.join(self.root_dir, ofile)
        assets = requests.get(api_url).json()["assets"]
        win_release = list(filter(lambda a: name_key_word in a["name"], assets))[0]
        with requests.get(win_release["browser_download_url"], stream=True) as r:
            with open(os.path.join(self.root_dir, output_file), "wb") as f:
                shutil.copyfileobj(r.raw, f)

        print("OK" if os.path.exists(output_file) else "ERROR")
        return os.path.exists(output_file)


os.chdir(root_dir)

extra_macros = [("VIRT_CUBE", None), ("DYNAMIC_SHADER_INCLUDE", None)]
led_module = Extension(
    "_ledcd",
    sources=["src/swig_module_wrap.cxx", "src/CubeDrawer.cpp"],
    define_macros=extra_macros,
    library_dirs=include_dirs
    # extra_link_args=os.environ["LEDCD_LIB_ARGS"].split(";"),
    # library_dirs=os.environ["LEDCD_INCLUDE_DIR"].split(";"),
)

setup(
    name="LedCD",
    version="0.1",
    author="dk731",
    description="""Led Cube driver module""",
    ext_modules=[led_module],
    py_modules=["_ledcd"],
    package_dir={"": "src"},
    cmdclass={"build": CustomBuild},
)
