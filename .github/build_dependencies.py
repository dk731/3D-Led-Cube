import os
import subprocess
import glob
import shutil
import urllib.request
import tarfile

root_dir = os.getcwd()
include_dir = os.path.join(root_dir, "include")
lib_dir = os.path.join(root_dir, "lib")

print("Prepearing folder structure...")
os.mkdir(os.path.join(root_dir, "lib"))


def build_cblas():
    os.chdir(root_dir)
    print("Building CBLAS...")
    subprocess.call(
        "git clone --depth 1 https://github.com/Reference-LAPACK/lapack-release",
        shell=True,
    )
    os.rename("./lapack-release/make.inc.example", "./lapack-release/make.inc")
    os.mkdir("./lapack-release/build")
    os.chdir("./lapack-release/build")

    subprocess.call(["cmake", "-DCBLAS=ON", "-DBUILD_SHARED_LIBS=ON", ".."])
    subprocess.call(["make", "cblas"])

    for file in glob.glob("./include/*"):
        shutil.move(file, include_dir)
    for file in glob.glob("./lib/*"):
        shutil.move(file, lib_dir)


def build_glew():
    os.chdir(root_dir)
    print("Building GLEW...")

    ftpstream = urllib.request.urlopen(
        "https://github.com/nigels-com/glew/releases/download/glew-2.2.0/glew-2.2.0.tgz"
    )
    file = tarfile.open(fileobj=ftpstream, mode="r|gz")
    file.extractall(path=".")

    os.chdir("glew-2.2.0")
    subprocess.call(["make", "glew.lib"])

    for file in glob.glob("./include/GL/*"):
        shutil.move(file, os.path.join(include_dir, "GL"))
    for file in glob.glob("./lib/*"):
        shutil.move(file, lib_dir)


def build_glfw():
    os.chdir(root_dir)
    print("Building GLFW...")
    subprocess.call(
        "git clone --depth 1 https://github.com/glfw/glfw",
        shell=True,
    )
    os.mkdir("./glfw/build")
    os.chdir("./glfw/build")
    subprocess.call(
        [
            "cmake",
            "-DBUILD_SHARED_LIBS=ON",
            "-DGLFW_BUILD_EXAMPLES=OFF",
            "-DGLFW_BUILD_TESTS=OFF",
            "-DGLFW_BUILD_DOCS=OFF",
            "-DGLFW_INSTALL=OFF",
            "..",
        ]
    )

    subprocess.call(["make"])

    for file in glob.glob("../include/*"):
        shutil.move(file, include_dir)
    for file in glob.glob("./src/libglfw*"):
        shutil.move(file, lib_dir)


def build_wsserver():
    os.chdir(root_dir)
    print("Building wsServer...")
    subprocess.call(
        "git clone --depth 1 https://github.com/Theldus/wsServer",
        shell=True,
    )
    os.chdir("./wsServer")
    os.environ["CFLAGS"] = "-fpic"
    subprocess.call(["make"])

    for file in glob.glob("./include/*"):
        shutil.move(file, include_dir)

    shutil.move("./libws.a", lib_dir)


build_cblas()
build_glew()
build_glfw()
build_wsserver()

os.chdir(root_dir)

print("Finished building dependencies!\n")
print("* Include dir: \n\n", "\n".join(glob.glob("./include/*")))
print("* Include/GL dir: \n\n", "\n".join(glob.glob("./include/GL/*")))
print("* Lib dir: \n\n", "\n".join(glob.glob("./lib/*")))
