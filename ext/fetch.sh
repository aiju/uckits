#!/bin/sh
wget ftp://ftp.gnu.org/gnu/binutils/binutils-2.29.1.tar.xz
tar xJf binutils-2.29.1.tar.xz
wget ftp://ftp.gnu.org/gnu/gcc/gcc-7.2.0/gcc-7.2.0.tar.xz
tar xJf gcc-7.2.0.tar.xz
cd gcc-7.2.0
contrib/download_prerequisites 
cd ..
patch -p0 < t-arm-elf.patch
