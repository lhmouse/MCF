@echo off

set Config=Debug32
set CXXFlags=-fno-builtin -g -O0
if "%1"=="Release" (
	set Config=Release32
	set CXXFlags=-DNDEBUG -O3 -ffunction-sections -fdata-sections -Wl,--gc-sections,-s
)

call mingw 32

mcfbuild -p../../MCFCRT/MCFBuild.mcfproj -s../../MCFCRT -d.%Config%/mcfcrt -o.%Config%/libmcfcrt.a %*
mcfbuild -p../../MCF/MCFBuild.mcfproj -s../../MCF -d.%Config%/mcf -o.%Config%/libmcf.a %*

g++ %CXXFlags% -std=c++11 -Wall -Wextra -pipe -mfpmath=sse,387 -msse2 -masm=intel main.cpp -o ".%Config%/a.exe" -I../.. -L.%Config% -static -nostartfiles -Wl,-e__MCFExeStartup,--disable-runtime-pseudo-reloc,--disable-auto-import,-lmcf,-lmcfcrt,-lstdc++,-lgcc,-lgcc_eh,-lmingwex,-lmcfcrt
