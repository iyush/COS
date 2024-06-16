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

mkdir -p $SRCDIR
mkdir -p $BUILDDIR

BINUTILS_FILE_PATH=$SRCDIR/binutils-$BINUTILS_VERSION.tar.xz
if [[ -e "$BINUTILS_FILE_PATH" ]]; then
  echo "We have already downloaded binutils-$BINUTILS_VERSION in $BINUTILS_FILE_PATH"
else
  echo "We are downloading binutils-$BINUTILS_VERSION to $BINUTILS_FILE_PATH"
  wget "https://ftp.gnu.org/gnu/binutils/binutils-$BINUTILS_VERSION.tar.xz" -O $BINUTILS_FILE_PATH
fi

GCC_FILE_PATH=$SRCDIR/gcc-$GCC_VERSION.tar.xz
if [[ -e "$GCC_FILE_PATH" ]]; then
  echo "We have already downloaded gcc-$GCC_VERSION to $GCC_FILE_PATH"
else
  echo "We are downloading gcc-$GCC_VERSION to $GCC_FILE_PATH"
  wget "https://ftp.gnu.org/gnu/gcc/gcc-$GCC_VERSION/gcc-$GCC_VERSION.tar.xz" -O $GCC_FILE_PATH
fi

# extracting
tar xvf $BINUTILS_FILE_PATH -C $SRCDIR
tar xvf $GCC_FILE_PATH -C $SRCDIR


# building binutils and gcc
mkdir -p $BUILDDIR/build-binutils &&
  cd $BUILDDIR/build-binutils &&
  $SRCDIR/binutils-$BINUTILS_VERSION/configure --target=$TARGET --prefix=$PREFIX --with-sysroot --disable-nls --disable-werror &&
  make -j8 &&
  make install &&

  mkdir -p $BUILDDIR/build-gcc &&
  cd $BUILDDIR/build-gcc &&
  $SRCDIR/gcc-$GCC_VERSION/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c --without-headers &&
  make all-gcc -j8            &&
  make all-target-libgcc -j8  &&
  make install-gcc            &&
  make install-target-libgcc 
