#!/bin/bash

CC=gcc
DEST=../_Build/stage1

rm -rf "${DEST}/"

mkdir -p "${DEST}/bin/"
mkdir -p "${DEST}/lib/"
mkdir -p "${DEST}/include/mcfbuild/"

"${CC}" -std=c11 -Wall -Wextra -pedantic -pedantic-errors -Werror \
  -Wno-error=unused-parameter -Winvalid-pch -Wwrite-strings -Wconversion \
  -Wsign-conversion -Wdouble-promotion -Wsuggest-attribute=noreturn \
  -Wundef -Wshadow -Wstrict-aliasing=2 -Wstrict-prototypes \
  -pipe -mfpmath=both -march=core2 -mtune=intel \
  -shared -Wl,--export-all-symbols,--exclude-libs,ALL \
  -Wl,--disable-stdcall-fixup,--enable-auto-image-base \
  -O0 -g $(find . -name "*.c") \
  -o "${DEST}/bin/mcfbuild-stage1.dll" \
  -Wl,--out-implib,"${DEST}/lib/libmcfbuild.dll.a"
cp -Rp $(find . -name "*.h") \
  -t "${DEST}/include/mcfbuild/" --parents
