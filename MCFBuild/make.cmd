@echo off

set CompilerFlags=-Wall -Wextra -pipe -mfpmath=sse,387 -msse2 -masm=intel
set LinkerFlags=-Wall -Wextra -static
set ExtraLibs=-lversion -lshlwapi

set CFlags=%CompilerFlags% -std=c11
set CPPFlags=%CompilerFlags% -std=c++11

if not "%1"=="Release" (
	set CompilerFlags=%CompilerFlags% -fno-builtin -g -O0
	set LinkerFlags=%LinkerFlags% -O0
) else (
	set CompilerFlags=%CompilerFlags% -DNDEBUG -O3 -ffunction-sections -fdata-sections
	set LinkerFlags=%LinkerFlags% -O3 -Wl,--gc-sections,-s
)

windres -i MCFBuild.rc -o "%temp%\MCFBuild.rc.o" -O coff
g++ %CPPFlags% *.cpp "%temp%\MCFBuild.rc.o" %LinkerFlags% %ExtraLibs%
