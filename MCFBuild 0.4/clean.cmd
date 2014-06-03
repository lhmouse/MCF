@echo off

set Config=Debug

set CompilerFlags=-Wall -Wextra -Wsign-conversion -Wsuggest-attribute=noreturn -pipe -mfpmath=sse,387 -march=core2 -masm=intel -D__MCF_CRT_NO_DLMALLOC
set LinkerFlags=-Wall -Wextra -static -nostartfiles -Wl,-e__MCF_ExeStartup,--disable-runtime-pseudo-reloc,--disable-auto-import

if not "%1"=="Release" (
	set CompilerFlags=%CompilerFlags% -fno-builtin -g -O0
	set LinkerFlags=%LinkerFlags% -O0
) else (
	set Config=Release
	set CompilerFlags=%CompilerFlags% -DNDEBUG -O3 -ffunction-sections -fdata-sections
	set LinkerFlags=%LinkerFlags% -O3 -Wl,--gc-sections,-s
)

set CFlags=%CompilerFlags% -std=c11
set CPPFlags=%CompilerFlags% -std=c++1y

set TempDir=.Temp-%Config%
set ExeFile=.%Config%.exe

echo ----- Configurations
echo   CFlags      : %CFlags%
echo   CPPFlags    : %CPPFlags%
echo   LinkerFlags : %LinkerFlags%

echo ----- Cleaning up...
md "%TempDir%" >nul 2>nul
md "%TempDir%\libmcflite" >nul 2>nul
del "%TempDir%\*.*" /f /s /a /q 2>nul

echo ----- Building libmcflite.a...
for /f "eol=;" %%I in (libmcflite.lst) do (
	echo %%I
	if "%%~xI"==".c" (
		gcc %CFlags% "%%I" -c -o "%TempDir%\libmcflite\%%~nxI.o" || exit /b 1
	) else (
		g++ %CPPFlags% "%%I" -c -o "%TempDir%\libmcflite\%%~nxI.o" || exit /b 1
	)
)
ar rcs "%TempDir%\libmcflite.a" "%TempDir%\libmcflite\*.o"
