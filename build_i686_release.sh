#!/bin/sh

set -e

prefix="$(pwd)/release/mingw32"
mkdir -p "$prefix"

export CPPFLAGS="-I$prefix/include -DNDEBUG"
export CFLAGS="-O3 -ffunction-sections -fdata-sections"
export CXXFLAGS="-O3 -ffunction-sections -fdata-sections"
export LDFLAGS="-O3 -Wl,-s,--gc-sections -L$prefix/lib"

builddir="$(pwd)/.build_i686_release"
build=i686-w64-mingw32

mkdir -p "$builddir"

(cd MCFCRT &&
  mkdir -p m4
  autoreconf -i)
(mkdir -p "$builddir/MCFCRT" && cd "$builddir/MCFCRT" &&
  ../../MCFCRT/configure --build="$build" --host="$build" --prefix="$prefix" &&
  make -j7 &&
  make install)

(cd MCF &&
  mkdir -p m4
  autoreconf -i)
(mkdir -p "$builddir/MCF" && cd "$builddir/MCF" &&
  ../../MCF/configure --build="$build" --host="$build" --prefix="$prefix" &&
  make -j7 &&
  make install)
