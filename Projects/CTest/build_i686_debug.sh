#!/bin/sh

CPPFLAGS+=" -Og -g -Wall -Wextra -pedantic -pedantic-errors -Wno-error=unused-parameter -Winvalid-pch	\
	-Wwrite-strings -Wconversion -Wsign-conversion -Wsuggest-attribute=noreturn -Wundef -Wshadow -Wstrict-aliasing=2 -Wstrict-overflow=5	\
	-pipe -mfpmath=both -march=nocona -mno-stack-arg-probe -mno-accumulate-outgoing-args -mpush-args -masm=intel
	-I../../debug/mingw32/include"
CFLAGS+=" -Og -g -std=c11 -Wstrict-prototypes"
LDFLAGS+=" -Og -g -nostdlib -L../../debug/mingw32/lib -static -lmcf -lmingwex -lgcc -lgcc_eh -static -lmcfcrt-pre-exe -static -lmcfcrt -lmsvcrt -lkernel32 -lntdll -Wl,--disable-runtime-pseudo-reloc,-e@__MCFCRT_ExeStartup"

cp -fp ../../debug/mingw32/bin/*.dll ./

i686-w64-mingw32-gcc ${CPPFLAGS} ${CFLAGS} main.c ${LDFLAGS}
