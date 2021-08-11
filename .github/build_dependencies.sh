echo "Prepering folder structure..."
root_dir=$(pwd)
mkdir lib

echo "Building GLFW..."
git clone https://github.com/glfw/glfw
cd glfw
mkdir build
cd build
cmake -DBUILD_SHARED_LIBS=ON -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF -DGLFW_INSTALL=OFF ..
make

cat ./CMakeFiles/CMakeOutput.log
cat ./CMakeFiles/CMakeError.log

mv ../include/* ${root_dir}/include
mv ./src/libglfw* ${root_dir}/lib

exit 0

echo "Building CBLAS..."
git clone --depth 1 https://github.com/Reference-LAPACK/lapack-release
cd lapack-release
mv make.inc.example make.inc
mkdir build
cd build
cmake -DCBLAS=ON -DBUILD_SHARED_LIBS=ON ..
make cblas

mv ./lib/* ${root_dir}/lib
mv ./include/* ${root_dir}/include
cd ${root_dir}

echo "Building GLEW..."
if curl -L https://github.com/nigels-com/glew/releases/download/glew-2.2.0/glew-2.2.0.tgz > glew.tgz; then
    echo "Downloaded file"
    tar -xf glew.tgz
    ls -la
else
    printf 'Curl failed with error code "%d" (check the manual)\n' "$?" >&2
    exit 1
fi

cd glew-2.2.0
make glew.lib
mv ./include/GL/* ${root_dir}/include/GL
mv ./lib/* ${root_dir}/lib

cd ${root_dir}

