#!/bin/sh

MCF_DEBUG=1

if [[ -z $CC ]]
then
	export CC=gcc
fi

if [[ -z $AR ]]
then
	export AR=ar
fi

export MCF_CPPFLAGS="-Wall -Wextra -pedantic -Wsign-conversion -Wsuggest-attribute=noreturn -Wundef -pipe -mfpmath=both -march=nocona -masm=intel"
export MCF_CFLAGS="-std=c11"
export MCF_CXXFLAGS="-std=c++14 -Wzero-as-null-pointer-constant -Wnoexcept"

if [[ $MCF_DEBUG ]]
then
	MCF_CPPFLAGS+=" -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -O0 -g -fno-builtin"
else
	MCF_CPPFLAGS+=" -DNDEBUG -O3 -ffunction-sections -fdata-sections -Wl,--gc-sections,-s"
fi

mkdir -p ".tmp"

if [ ! -f .tmp/libmcflite.a ]
then
	(cd sh && ./buildlite.sh)
fi
(cd sh && ./build.sh)
