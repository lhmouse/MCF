#!/bin/sh

CPPFLAGS+=" -O3 -g -DNDEBUG -Wall -Wextra -pedantic -pedantic-errors -Werror -Wno-error=unused-parameter	\
	-Wwrite-strings -Wconversion -Wsign-conversion -Wsuggest-attribute=noreturn -Wundef -Wshadow -Wstrict-aliasing=2 -Wstrict-overflow=5	\
	-pipe -mfpmath=both -march=nocona -mno-stack-arg-probe -mno-accumulate-outgoing-args -mpush-args -masm=intel	\
	-I../../release/mingw64/include"
CXXFLAGS+=" -std=c++14 -Wzero-as-null-pointer-constant -Wnoexcept -Woverloaded-virtual -fnothrow-opt"
LDFLAGS+=" -static -nostdlib -L../../release/mingw64/lib -lmcf -lsupc++ -lmingwex -lgcc -lgcc_eh -lmcfcrt-pre-exe -lmcfcrt -lmsvcrt -lkernel32 -lntdll -Wl,-e@__MCFCRT_ExeStartup"

cp -fp ../../release/mingw64/bin/*.dll ./

x86_64-w64-mingw32-g++ ${CPPFLAGS} ${CXXFLAGS} main.cpp ${LDFLAGS}
