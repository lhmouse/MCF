#!/bin/sh

CPPFLAGS+=" -Og -g -Wall -Wextra -pedantic -pedantic-errors -Wno-error=unused-parameter -Winvalid-pch	\
	-Wwrite-strings -Wconversion -Wsign-conversion -Wsuggest-attribute=noreturn -Wundef -Wshadow -Wstrict-aliasing=2 -Wstrict-overflow=5	\
	-pipe -mfpmath=both -march=nocona -mno-stack-arg-probe -mno-accumulate-outgoing-args -mpush-args -masm=intel	\
	-I../../debug/mingw64/include"
CFLAGS+=" -Og -g -std=c11 -Wstrict-prototypes"
LDFLAGS+=" -Og -nostdlib -L../../debug/mingw64/lib -lmcf -lmingwex -lgcc -lmcfcrt-pre-exe -lmcfcrt -lmsvcrt -lkernel32 -lntdll -Wl,-e@__MCFCRT_ExeStartup"

cp -fp ../../debug/mingw64/bin/*.dll ./

x86_64-w64-mingw32-gcc ${CPPFLAGS} ${CFLAGS} main.c ${LDFLAGS}
