import os
import subprocess
import glob
import shutil
import urllib.request
import tarfile
from itertools import chain

root_dir = os.getcwd()
include_dir = os.path.join(root_dir, "include")
lib_dir = os.path.join(root_dir, "lib")

gl_dir = os.path.join(include_dir, "GL")

os.environ["LD_LIBRARY_PATH"] = lib_dir

print("Prepearing folder structure...")
dirs_to_delete = [
    "glfw",
    "include/GLFW",
    "glm",
    "include/glm",
    "glew-2.2.0",
    "wsServer",
    "lib",
]

clear_exceptions = ["gl.h", "glu.h"]
dirs_to_clear = ["include", "include/GL"]

for d in dirs_to_clear:
    files = os.listdir(d)
    for file in files:
        full_path = os.path.join(d, file)
        if file not in clear_exceptions and os.path.isfile(full_path):
            os.remove(full_path)

for d in dirs_to_delete:
    if os.path.exists(os.path.join(root_dir, d)):
        shutil.rmtree(d, ignore_errors=True)

os.mkdir(lib_dir)


def build_glm():
    os.chdir(root_dir)
    print("Building GLM...")
    subprocess.call(
        "git clone --depth 1 https://github.com/g-truc/glm",
        shell=True,
    )

    shutil.move("./glm/glm", include_dir)


def build_glew():
    os.chdir(root_dir)
    print("Building GLEW...")

    ftpstream = urllib.request.urlopen(
        "https://github.com/nigels-com/glew/releases/download/glew-2.2.0/glew-2.2.0.tgz"
    )
    file = tarfile.open(fileobj=ftpstream, mode="r|gz")
    file.extractall(path=".")

    os.chdir("glew-2.2.0")
    subprocess.call(["make", "WARN=-Wall -Wno-cast-function-type", "glew.lib.shared"])

    for file in glob.glob("./include/GL/*"):
        shutil.move(file, os.path.join(include_dir, "GL"))
    for file in glob.glob("./lib/*"):
        shutil.move(file, lib_dir)


def build_glfw():
    os.chdir(root_dir)
    print("Building GLFW...")
    subprocess.call(
        "git clone https://github.com/glfw/glfw",
        shell=True,
    )
    os.chdir("./glfw")
    subprocess.call("git checkout 201400b974b63eb7f23eb7d8563589df9c699d7c", shell=True)
    os.mkdir("./build")
    os.chdir("./build")

    subprocess.call(
        [
            "cmake",
            "-DBUILD_SHARED_LIBS=ON",
            "-DGLFW_BUILD_EXAMPLES=OFF",
            "-DGLFW_BUILD_TESTS=OFF",
            "-DGLFW_BUILD_DOCS=OFF",
            "-DGLFW_INSTALL=OFF",
            f"-DCMAKE_RUNTIME_OUTPUT_DIRECTORY={lib_dir}",
            f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={lib_dir}",
            "..",
        ]
    )

    print("Current build dir: ", os.listdir("."))

    subprocess.call("cmake --build .", shell=True)

    for file in glob.glob("../include/*"):
        shutil.move(file, include_dir)
    # for file in glob.glob("./src/libglfw*"):
    #     shutil.move(file, lib_dir)


def build_wsserver():
    os.chdir(root_dir)
    print("Building wsServer...")
    subprocess.call(
        "git clone --depth 1 https://github.com/Theldus/wsServer",
        shell=True,
    )
    os.mkdir("./wsServer/build")
    os.chdir("./wsServer/build")
    # os.chdir("./wsServer")
    os.environ["CFLAGS"] = "-fPIC"
    subprocess.call(
        'cmake -DCMAKE_C_FLAGS=-fPIC -DCMAKE_BUILD_TYPE=Release -G "MinGW Makefiles" ..',
        shell=True,
    )
    subprocess.call("mingw32-make -j4", shell=True)
    # subprocess.call("make libws.a", shell=True)

    for file in glob.glob("../include/*"):
        shutil.move(file, include_dir)

    shutil.move("./libws.a", lib_dir)


# build_glm()
# build_glew()
# build_glfw()
build_wsserver()

os.chdir(root_dir)
for d, _, files in list(os.walk(lib_dir))[1:]:
    for file in files:
        shutil.move(os.path.join(root_dir, d, file), lib_dir)

print("Finished building dependencies!\n")
print("* Include dir: \n\n", "\n".join(glob.glob("./include/*")))
print("* Include/GL dir: \n\n", "\n".join(glob.glob("./include/GL/*")))
print("* Lib dir: \n\n", "\n".join(glob.glob("./lib/*")))

print(
    "Environment variables: \n",
    "\n".join([f"{name}: {val}" for name, val in os.environ.items()]),
)

print()

print()
