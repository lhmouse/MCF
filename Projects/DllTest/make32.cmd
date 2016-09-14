@echo off

set Config=Debug32
set CXXFlags=-fno-builtin -g -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -O0
if "%1"=="Release" (
	set Config=Release32
	set CXXFlags=-DNDEBUG -O3 -ffunction-sections -fdata-sections -Wl,--gc-sections,-s
)

set Lib=-lmcf -static -lmcfcrt -lstdc++ -lgcc -lgcc_eh -static -lmcfcrt -lgcc -lmsvcrt -lmsvcrt -lkernel32 -lntdll -luser32 -lshell32 -ladvapi32

call mingw 32

mcfbuild -p../../MCFCRT/MCFBuild.mcfproj -s../../MCFCRT -d../../.Built/%Config%/mcfcrt -o../../.Built/%Config%/libmcfcrt.a %* || exit /b 1
mcfbuild -p../../MCF/MCFBuild.mcfproj -s../../MCF -d../../.Built/%Config%/mcf -o../../.Built/%Config%/libmcf.a %* || exit /b 1
mcfbuild -p../../External/zlib/MCFBuild.mcfproj -s../../External/zlib -d../../.Built/%Config%/zlib -o../../.Built/%Config%/libz.a %* || exit /b 1

g++ %CXXFlags% -std=c++17 -Wnoexcept -Woverloaded-virtual -fnothrow-opt -Wall -Wextra -pedantic -pedantic-errors -Wwrite-strings -Wsign-conversion -Wsuggest-attribute=noreturn -pipe -mfpmath=both -march=nocona -mno-stack-arg-probe -mno-accumulate-outgoing-args -mpush-args -masm=intel dll.cpp -shared -o ".dll-%Config%.dll" -I../.. -L../../.Built/%Config% -static -nostdlib -Wl,-e@__MCFCRT_DllStartup,--disable-runtime-pseudo-reloc %Lib% || exit /b 1

g++ dlltest.cpp ".dll-%Config%.dll" -std=c++17 -o ".%Config%.exe" -static -O3 -Wl,-s
