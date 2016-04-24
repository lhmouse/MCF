@echo off

set Config=Debug32
set CXXFlags=-fno-builtin -g -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -O0
if "%1"=="Release" (
	set Config=Release32
	set CXXFlags=-DNDEBUG -O3 -flto -ffunction-sections -fdata-sections -Wl,--gc-sections,-s
)

set Lib=-lmcf -lmcfcrt -lsupc++ -lmingwex -lmingw32 -lgcc -lgcc_eh -lgcc -lmingwex -lmcfcrt -lmsvcrt -lkernel32 -lntdll -luser32 -lshell32 -ladvapi32

call mingw 32

mcfbuild -p../../MCFCRT/MCFBuild.mcfproj -s../../MCFCRT -d../../.Built/%Config%/mcfcrt -o../../.Built/%Config%/libmcfcrt.a %* || exit /b 1
mcfbuild -p../../MCF/MCFBuild.mcfproj -s../../MCF -d../../.Built/%Config%/mcf -o../../.Built/%Config%/libmcf.a %* || exit /b 1
:: mcfbuild -p../../External/zlib/MCFBuild.mcfproj -s../../External/zlib -d../../.Built/%Config%/zlib -o../../.Built/%Config%/libz.a %* || exit /b 1
:: mcfbuild -p../../External/lzmalite/MCFBuild.mcfproj -s../../External/lzmalite -d../../.Built/%Config%/lzmalite -o../../.Built/%Config%/liblzmalite.a %* || exit /b 1

g++ %CXXFlags% -std=c++14 -Wnoexcept -Woverloaded-virtual -fnothrow-opt -Wall -Wextra -pedantic -pedantic-errors -Wsign-conversion -Wsuggest-attribute=noreturn -pipe -mfpmath=both -march=nocona -mno-stack-arg-probe -mno-accumulate-outgoing-args -mpush-args -masm=intel main.cpp -o ".%Config%.exe" -I../.. -L../../.Built/%Config% -static -nostdlib -Wl,-e__MCFCRT_ExeStartup,--disable-runtime-pseudo-reloc,--disable-auto-import,--wrap=atexit,--wrap=abort,--wrap=malloc,--wrap=realloc,--wrap=calloc,--wrap=free %Lib% || exit /b 1
