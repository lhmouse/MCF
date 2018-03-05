#!/bin/bash

_cc="gcc"
_dest="../_Build/stage1"

if [[ -n "${OPTIMIZE}" ]]; then
  _opt_flags="-O2"
else
  _opt_flags="-O0"
fi

#rm -rf "${_dest}/"

mkdir -p "${_dest}/bin/"
mkdir -p "${_dest}/lib/"
mkdir -p "${_dest}/include/mcfbuild/"

"${_cc}" -std=c11 -Wall -Wextra -pedantic -pedantic-errors -Werror \
  -Wno-error=unused-parameter -Winvalid-pch -Wwrite-strings -Wconversion \
  -Wsign-conversion -Wdouble-promotion -Wsuggest-attribute=noreturn \
  -Wundef -Wshadow -Wstrict-aliasing=2 -Wstrict-prototypes \
  -pipe -mfpmath=both -march=native -mtune=intel -municode \
  -shared -Wl,--export-all-symbols,--exclude-libs,ALL \
  -Wl,--disable-stdcall-fixup,--enable-auto-image-base \
  "${_opt_flags}" -g3 *.c \
  -o "${_dest}/bin/mcfbuild-stage1.dll" \
  -Wl,--out-implib,"${_dest}/lib/libmcfbuild.dll.a"
cp -Rp *.h \
  -t "${_dest}/include/mcfbuild/" --parents
