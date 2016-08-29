#!/bin/sh

CPPFLAGS+=" -Wall -Wextra -pedantic -pedantic-errors -Werror -Wno-error=unused-parameter -Winvalid-pch	\
	-Wwrite-strings -Wconversion -Wsign-conversion -Wsuggest-attribute=noreturn -Wundef -Wshadow -Wstrict-aliasing=2 -Wstrict-overflow=5	\
	-pipe -mfpmath=both -march=native -mno-stack-arg-probe -mno-accumulate-outgoing-args -mpush-args -masm=intel	\
	-I../../debug/mingw64/include"
CFLAGS+=" -std=c11 -Wstrict-prototypes"
LDFLAGS+=" -static -nostdlib -L../../debug/mingw64/lib -lmcf -lsupc++ -lmingwex -lgcc -lgcc_eh -lmcfcrt-pre-exe -lmcfcrt -lmsvcrt -lkernel32 -lntdll -Wl,-e@__MCFCRT_ExeStartup"

cp -fp ../../debug/mingw64/bin/*.dll ./

x86_64-w64-mingw32-gcc ${CPPFLAGS} ${CFLAGS} main.c ${LDFLAGS}
