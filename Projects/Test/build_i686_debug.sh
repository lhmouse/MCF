#!/bin/sh

CPPFLAGS+=" -O0 -g -Wall -Wextra -pedantic -pedantic-errors -Werror -Wno-error=unused-parameter	\
	-Wwrite-strings -Wconversion -Wsign-conversion -Wsuggest-attribute=noreturn -Wundef -Wshadow -Wstrict-aliasing=2 -Wstrict-overflow=5	\
	-pipe -mfpmath=both -march=nocona -mno-stack-arg-probe -mno-accumulate-outgoing-args -mpush-args -masm=intel
	-I../../debug/mingw32/include"
CXXFLAGS+=" -std=c++14 -Wzero-as-null-pointer-constant -Wnoexcept -Woverloaded-virtual -fnothrow-opt"
LDFLAGS+=" -static -nostdlib -L../../debug/mingw32/lib -lmcf -lsupc++ -lmingwex -lgcc -lgcc_eh -lmcfcrt-pre-exe -lmcfcrt -lmsvcrt -lkernel32 -lntdll -Wl,-e@__MCFCRT_ExeStartup"

cp -fp ../../debug/mingw32/bin/*.dll ./

i686-w64-mingw32-g++ ${CPPFLAGS} ${CXXFLAGS} main.cpp ${LDFLAGS}
