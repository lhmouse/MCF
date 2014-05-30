@echo off

set Config=Debug

set CompilerFlags=-Wall -Wextra -Wsign-conversion -Wsuggest-attribute=noreturn -pipe -mfpmath=sse,387 -msse2 -masm=intel
set LinkerFlags=-Wall -Wextra -static
set ExtraLibs=-lshlwapi

if not "%1"=="Release" (
	set CompilerFlags=%CompilerFlags% -fno-builtin -g -O0
	set LinkerFlags=%LinkerFlags% -O0
) else (
	set Config=Release
	set CompilerFlags=%CompilerFlags% -DNDEBUG -O3 -ffunction-sections -fdata-sections -flto
	set LinkerFlags=%LinkerFlags% -O3 -Wl,--gc-sections,-s -flto
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
md %TempDir% >nul 2>nul
del %TempDir%\*.o /f /s /a 2>nul

echo ----- Compiling pre-compiled header...
echo MCFBuild.hpp
g++ %CPPFlags% "MCFBuild.hpp" -o "%TempDir%\MCFBuild.hpp.gch" || exit /b 1
echo #warning Failed to load precompiled header file. > %TempDir%\MCFBuild.hpp
echo #include "../MCFBuild.hpp" >> %TempDir%\MCFBuild.hpp

echo ----- Compiling MCFBuild...
for /f %%I in ('dir *.c *.cpp /b /o:-d /t:w') do (
	echo %%I
	if "%%~xI"==".c" (
		gcc %CFlags% "%%I" -c -o "%TempDir%\%%~nxI.o" || exit /b 1
	) else (
		g++ %CPPFlags% "%%I" -include "%TempDir%\MCFBuild.hpp" -c -o "%TempDir%\%%~nxI.o" || exit /b 1
	)
)

echo ----- Compiling MCF components...
for /f "eol=;" %%I in (make.lst) do (
	echo %%I
	if "%%~xI"==".c" (
		gcc %CFlags% "%%I" -c -o "%TempDir%\%%~nxI.o" || exit /b 1
	) else (
		g++ %CPPFlags% "%%I" -c -o "%TempDir%\%%~nxI.o" || exit /b 1
	)
)

echo ----- Linking...
echo   =^> %ExeFile%
g++ "%TempDir%\*.o" %LinkerFlags% %ExtraLibs% -o "%ExeFile%" || exit /b 1

echo ----- Succeeded
