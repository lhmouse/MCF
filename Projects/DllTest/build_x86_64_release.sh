#!/bin/bash

CPPFLAGS=" -O3 -DNDEBUG -Wall -Wextra -pedantic -pedantic-errors -Wno-error=unused-parameter -Winvalid-pch	\
	-Wwrite-strings -Wconversion -Wsign-conversion -Wsuggest-attribute=noreturn -Wundef -Wshadow -Wstrict-aliasing=2 -Wstrict-overflow=5	\
	-pipe -mfpmath=both -march=core2 -mtune=intel -mno-stack-arg-probe -masm=intel	\
	-I../../release/mingw64/include"
CXXFLAGS=" -O3 -std=c++17 -Wzero-as-null-pointer-constant -Wnoexcept -Woverloaded-virtual -Wsuggest-override -fnothrow-opt"
LDFLAGS=" -shared -O3 -nostdlib -L../../release/mingw64/lib -lmcf -lstdc++ -lmcfcrt -lmingwex -lgcc -lgcc_s -lmcfcrt-pre-dll -lmcfcrt -lmsvcrt -lkernel32 -lntdll -Wl,-e@__MCFCRT_DllStartup"

cp -fp ../../release/mingw64/bin/*.dll ./

x86_64-w64-mingw32-g++ ${CPPFLAGS} ${CXXFLAGS} main.cpp ${LDFLAGS} -o test.dll
