#!/bin/sh

set -e

OBJS=

for SRC in $(cd "../src/" && echo *.cpp)
do
	DST="../.tmp/$(echo $SRC.o | sed 's,^.*/,,')"

	CMD="$CC $MCF_CPPFLAGS $CPPFLAGS $MCF_CXXFLAGS $CXXFLAGS -isystem ../../MCF/ -c -o $DST ../src/$SRC"

	echo $CMD
	$CMD || exit

	OBJS+=" $DST"
done

CMD="$CC $MCF_CPPFLAGS $CPPFLAGS $MCF_CXXFLAGS $CXXFLAGS -o ../.tmp/MCFBuild.exe $OBJS
	-Wall -Wextra -static -nostdlib -Wl,-e__MCF_ExeStartup,--disable-runtime-pseudo-reloc,--disable-auto-import
	-Wl,--wrap=atexit,--wrap=malloc,--wrap=realloc,--wrap=calloc,--wrap=free,--wrap=__cxa_throw
	-L../.tmp -lmcflite -lstdc++ -lmingwex -lmingw32 -lgcc -lgcc_eh -lmcflite -lmsvcrt -luser32 -lkernel32 $LDFLAGS $LIBADD"
echo $CMD
$CMD || exit
