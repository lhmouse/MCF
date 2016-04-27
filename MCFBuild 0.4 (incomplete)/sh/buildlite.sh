#!/bin/sh

set -e

OBJS=

for SRC in $(cat libmcflite.lst | sed 's,\s,,g')
do
	DST="../.tmp/$(echo $SRC.o | sed 's,^.*/,,')"

	CMD="$CC $MCF_CPPFLAGS $CPPFLAGS"
	if [[ -z $(echo $SRC | grep -E '\.cpp$') ]]
	then
		CMD+=" $MCF_CFLAGS $CFLAGS"
	else
		CMD+=" $MCF_CXXFLAGS $CXXFLAGS"
	fi
	CMD+=" -c -o $DST ../../$SRC"

	echo $CMD
	$CMD || exit

	OBJS+=" $DST"
done

CMD="$AR rcs ../.tmp/libmcflite.a "$OBJS
echo $CMD
$CMD || exit
