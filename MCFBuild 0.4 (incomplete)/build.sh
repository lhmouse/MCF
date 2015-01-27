#!/bin/sh

set -e

if [[ -z $CC ]]
then
	CC=gcc
fi

LIBMCFLITE_DIR=.lib

MCF_CPPFLAGS="-Wall -Wextra -pedantic -Wsign-conversion -Wsuggest-attribute=noreturn -Wundef -pipe -mfpmath=both -march=nocona -masm=intel"
MCF_CFLAGS="-std=c11"
MCF_CXXFLAGS="-std=c++14 -Wzero-as-null-pointer-constant -Wnoexcept"

MCF_LDFLAGS="-Wall -Wextra -static -nostdlib -Wl,-e__MCF_ExeStartup,--disable-runtime-pseudo-reloc,--disable-auto-import
	-Wl,--wrap=atexit,--wrap=malloc,--wrap=realloc,--wrap=calloc,--wrap=free,--wrap=__cxa_throw
	-L$LIBMCFLITE_DIR -lmcflite -lstdc++ -lmingwex -lmingw32 -lgcc -lgcc_eh -lmcflite -lmsvcrt -luser32 -lkernel32"

CMD="$CC $MCF_CPPFLAGS $CPPFLAGS $MCF_CXXFLAGS $CXXFLAGS -o MCFBuild.exe -isystem ../MCF/ "*.cpp" $MCF_LDFLAGS"
echo $CMD
$CMD || exit
