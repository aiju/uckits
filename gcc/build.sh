BINSRC=../ext/binutils-2.29.1
GCCSRC=../ext/gcc-7.2.0
JOBS=4

mkdir -p build-binutils
cd build-binutils
../$BINSRC/configure --target=arm-none-eabi --prefix=$(pwd)/.. --disable-libssp --disable-nls --disable-werror --disable-libstdcxx
make -j$JOBS
make install
cd ../build-binutils

mkdir -p build-gcc
cd build-gcc
../$GCCSRC/configure --target=arm-none-eabi --prefix=$(pwd)/.. --enable-languages=c --enable-multilib --disable-nls --disable-libssp --disable-shared --without-headers
make -j$JOBS all-gcc all-target-libgcc
make install-gcc install-target-libgcc
