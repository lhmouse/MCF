#!/bin/sh

set -e

prefix="$(pwd)/debug/mingw32"
mkdir -p "$prefix"

builddir="$(pwd)/build_i686_debug"
mkdir -p "$builddir"

build=i686-w64-mingw32

(cd MCFCRT &&
  mkdir -p m4
  autoreconf -i)

(cd "$builddir" &&
  CPPFLAGS=''	\
  CFLAGS='-O0 -g'	\
  LDFLAGS='-O0'	\
  ../MCFCRT/configure --build="$build" --host="$build" --prefix="$prefix" &&
  make -j4 &&
  make install)
