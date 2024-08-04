#!/usr/bin/env bash

set -e

TOOLCHAIN_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

echo $SCRIPT_DIR

BUILDDIR="$TOOLCHAIN_DIR/build"
SRCDIR="$TOOLCHAIN_DIR/src"
PREFIX="$TOOLCHAIN_DIR/build"
TARGET=x86_64-elf

BINUTILS_VERSION=2.40
GCC_VERSION=13.1.0
NASM_VERSION=2.16.03
XORRISO_VERSION=1.5.6
BOCHS_VERSION=2.8


download() {
        URL=$1
        FPATH=$2
        if [[ -e "$FPATH" ]]; then
          echo "We have already downloaded to $FPATH"
        else
          echo "We are downloading $URL to $FPATH"
          wget $URL -O $FPATH
        fi
}

mkdir -p $SRCDIR
mkdir -p $BUILDDIR

BINUTILS_FILE_PATH=$SRCDIR/binutils-$BINUTILS_VERSION.tar.xz
GCC_FILE_PATH=$SRCDIR/gcc-$GCC_VERSION.tar.xz
NASM_FILE_PATH=$SRCDIR/nasm-$NASM_VERSION.tar.xz
XORRISO_FILE_PATH=$SRCDIR/xorriso-$XORRISO_VERSION.tar.xz
BOCHS_FILE_PATH=$SRCDIR/bochs-$BOCHS_VERSION.tar.xz

download "https://ftp.gnu.org/gnu/binutils/binutils-$BINUTILS_VERSION.tar.xz"                                       $BINUTILS_FILE_PATH
download "https://ftp.gnu.org/gnu/gcc/gcc-$GCC_VERSION/gcc-$GCC_VERSION.tar.xz"                                     $GCC_FILE_PATH
download "https://www.nasm.us/pub/nasm/releasebuilds/$NASM_VERSION/nasm-$NASM_VERSION.tar.xz"                       $NASM_FILE_PATH
download "https://ftp.gnu.org/gnu/xorriso/xorriso-$XORRISO_VERSION.tar.gz"                                          $XORRISO_FILE_PATH
download "https://sourceforge.net/projects/bochs/files/bochs/$BOCHS_VERSION/bochs-$BOCHS_VERSION.tar.gz/download"   $BOCHS_FILE_PATH

# extracting
tar xf $BINUTILS_FILE_PATH -C $SRCDIR
tar xf $GCC_FILE_PATH -C $SRCDIR
tar xf $NASM_FILE_PATH -C $SRCDIR
tar xf $XORRISO_FILE_PATH -C $SRCDIR
tar xf $BOCHS_FILE_PATH -C $SRCDIR


# building binutils and gcc
mkdir -p $BUILDDIR/build-binutils 
cd $BUILDDIR/build-binutils 
$SRCDIR/binutils-$BINUTILS_VERSION/configure --target=$TARGET --prefix=$PREFIX --with-sysroot --disable-nls --disable-werror 
make -j8 
make install 

mkdir -p $BUILDDIR/build-gcc 
cd $BUILDDIR/build-gcc 
$SRCDIR/gcc-$GCC_VERSION/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c --without-headers 
make all-gcc -j8            
make all-target-libgcc -j8  
make install-gcc            
make install-target-libgcc 

mkdir -p $BUILDDIR/build-nasm 
cd $BUILDDIR/build-nasm 
$SRCDIR/nasm-$NASM_VERSION/configure --target=$TARGET --prefix="$PREFIX"
make all
make install

mkdir -p $BUILDDIR/build-nasm 
cd $BUILDDIR/build-nasm 
$SRCDIR/nasm-$NASM_VERSION/configure --target=$TARGET --prefix="$PREFIX"
make all
make install

mkdir -p $BUILDDIR/build-xorriso 
cd $BUILDDIR/build-xorriso 
$SRCDIR/xorriso-$XORRISO_VERSION/configure --target=$TARGET --prefix="$PREFIX"
make
make install

mkdir -p $BUILDDIR/build-bochs 
cd $BUILDDIR/build-bochs 
$SRCDIR/bochs-$BOCHS_VERSION/configure --target=$TARGET --prefix="$PREFIX"  \
    --with-x11 \
    --enable-plugins \
    --enable-debugger \
    --enable-readline \
    --enable-idle-hack
make
make install