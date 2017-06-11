#!/bin/sh

CPPFLAGS+=" -O3 -DNDEBUG -Wall -Wextra -pedantic -pedantic-errors -Wno-error=unused-parameter -Winvalid-pch	\
	-Wwrite-strings -Wconversion -Wsign-conversion -Wsuggest-attribute=noreturn -Wundef -Wshadow -Wstrict-aliasing=2 -Wstrict-overflow=5	\
	-pipe -mfpmath=both -march=nocona -mno-stack-arg-probe  -masm=intel	\
	-I../../release/mingw64/include"
CFLAGS+=" -O3 -std=c11 -Wstrict-prototypes"
LDFLAGS+=" -O3 -nostdlib -L../../release/mingw64/lib -lmcf -lmingwex -lgcc -lmcfcrt-pre-exe -lmcfcrt -lmsvcrt -lkernel32 -lntdll -Wl,-e@__MCFCRT_ExeStartup"

cp -fp ../../release/mingw64/bin/*.dll ./

x86_64-w64-mingw32-gcc ${CPPFLAGS} ${CFLAGS} main.c ${LDFLAGS}
