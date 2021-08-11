echo "Prepering folder structure..."
root_dir=$(pwd)
mkdir include
mkdir lib

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
    ls -la
    tar -xf glew.tgz
else
    printf 'Curl failed with error code "%d" (check the manual)\n' "$?" >&2
    exit 1
fi

cd glew
make glew.lib
mv ./include/* ${root_dir}/include
mv ./lib/* ${root_dir}/lib

cd ${root_dir}
