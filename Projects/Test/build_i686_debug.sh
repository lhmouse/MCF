#!/bin/sh

CPPFLAGS+=" -Og -g -Wall -Wextra -pedantic -pedantic-errors -Wno-error=unused-parameter -Winvalid-pch	\
	-Wwrite-strings -Wconversion -Wsign-conversion -Wsuggest-attribute=noreturn -Wsuggest-override -Wundef -Wshadow -Wstrict-aliasing=2 -Wstrict-overflow=5	\
	-pipe -mfpmath=both -march=nocona -mno-stack-arg-probe -mno-accumulate-outgoing-args -mpush-args -masm=intel
	-I../../debug/mingw32/include"
CXXFLAGS+=" -Og -g -std=c++17 -Wzero-as-null-pointer-constant -Wnoexcept -Woverloaded-virtual -fnothrow-opt"
LDFLAGS+=" -Og -nostdlib -L../../debug/mingw32/lib -DMCF_IMPORT_RTTI -lmcf -lsupc++ -lmcfcrt -lmingwex -lgcc -lgcc_eh -lmcfcrt-pre-exe -lmcfcrt -lmsvcrt -lkernel32 -lntdll -Wl,--disable-runtime-pseudo-reloc,-e@__MCFCRT_ExeStartup"

cp -fp ../../debug/mingw32/bin/*.dll ./

i686-w64-mingw32-g++ ${CPPFLAGS} ${CXXFLAGS} main.cpp ${LDFLAGS}
