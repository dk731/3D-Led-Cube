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
# wget -O glew.tgz https://github.com/nigels-com/glew/releases/download/glew-2.2.0/glew-2.2.0.tgz
curl https://github.com/nigels-com/glew/releases/download/glew-2.2.0/glew-2.2.0.tgz --output glew.tgz
echo "Downloaded glew.tgz"
ls -la
tar -xf glew.tgz
echo "Unpacked glew.tgz"
cd glew
echo "I am in glew dir!"
ls -la
make glew.lib
echo "runned make file"
ls -la ./include
ls -la ./lib
mv ./include/* ${root_dir}/include
mv ./lib/* ${root_dir}/lib

ls -la ${root_dir}/include
ls -la ${root_dir}/lib

cd ${root_dir}
