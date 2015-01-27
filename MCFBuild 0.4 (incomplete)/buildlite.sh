#!/bin/sh

set -e

if [[ -z $CC ]]
then
	CC=gcc
fi

if [[ -z $AR ]]
then
	AR=ar
fi

LIBMCFLITE_DIR=.lib

MCF_CPPFLAGS="-Wall -Wextra -pedantic -Wsign-conversion -Wsuggest-attribute=noreturn -Wundef -pipe -mfpmath=both -march=nocona -masm=intel"
MCF_CFLAGS="-std=c11"
MCF_CXXFLAGS="-std=c++14 -Wzero-as-null-pointer-constant -Wnoexcept"

mkdir -p $LIBMCFLITE_DIR
rm -f .lib/libmcflite.a

OBJS=

for SRC in $(cat libmcflite.lst | sed 's,\s,,g')
do
	DST="$LIBMCFLITE_DIR/$(echo $SRC.o | sed 's,^.*/,,')"

	CMD="$CC $MCF_CPPFLAGS $CPPFLAGS -c -o $DST ../$SRC"
	if [[ -z $(echo $SRC | grep -E '\.cpp$') ]]
	then
		CMD+=" $MCF_CFLAGS $CFLAGS"
	else
		CMD+=" $MCF_CXXFLAGS $CXXFLAGS"
	fi

	echo $CMD
	$CMD || exit

	OBJS+=" $DST"
done

CMD="$AR rcs .lib/libmcflite.a "$OBJS
echo $CMD
$CMD || exit
