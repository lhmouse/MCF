#!/bin/sh

set -e

prefix="$(pwd)/.debug/mingw64"
mkdir -p "$prefix"

builddir="$(pwd)/.build_x86_64_debug"
mkdir -p "$builddir"

build=x86_64-w64-mingw32

(cd MCFCRT &&
  mkdir -p m4
  autoreconf -i)

(cd "$builddir" &&
  CPPFLAGS=''	\
  CFLAGS='-O0 -g'	\
  LDFLAGS='-O0'	\
  ../MCFCRT/configure --build="$build" --host="$build" --prefix="$prefix" &&
  make -j7 &&
  make install)
