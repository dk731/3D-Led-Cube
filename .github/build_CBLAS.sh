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