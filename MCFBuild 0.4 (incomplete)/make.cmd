@echo off

set Config=Debug

set CompilerFlags=-Wall -Wextra -Wsign-conversion -Wsuggest-attribute=noreturn -pipe -mfpmath=sse,387 -march=core2 -masm=intel -D__MCF_CRT_NO_DLMALLOC
set LinkerFlags=-Wall -Wextra -static -nostdlib -Wl,-e__MCF_ExeStartup,--disable-runtime-pseudo-reloc,--disable-auto-import,--wrap=atexit,--wrap=malloc,--wrap=realloc,--wrap=calloc,--wrap=free,--wrap=__cxa_throw
set Libs=-L"%TempDir%" -lmcfbuild -lstdc++ -lmingwex -lmingw32 -lgcc -lgcc_eh -lmcfbuild -lmsvcrt -lkernel32 -luser32 -lshell32 -ladvapi32

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

echo ----- Configuration
echo   CFlags      : %CFlags%
echo   CPPFlags    : %CPPFlags%
echo   LinkerFlags : %LinkerFlags%
echo   Libs        : %Libs%

echo ----- Cleaning up...
md "%TempDir%" >nul 2>nul
del "%TempDir%\*.o" /f /s /a /q
del "%TempDir%\*.a" /f /s /a /q

echo ----- Compiling...
for /f "eol=;" %%I in (libmcflite.lst) do (
	echo "%TempDir%\%%~nxI.o" ^<- "%%I"
	if "%%~xI"==".c" (
		start /b /belownormal gcc %CFlags% "%%I" -c -o "%TempDir%\%%~nxI.o" || exit /b 1
	) else (
		start /b /belownormal g++ %CPPFlags% "%%I" -c -o "%TempDir%\%%~nxI.o" || exit /b 1
	)
)

echo MCFBuild.hpp -> "%TempDir%\MCFBuild.hpp.gch"
g++ %CPPFlags% "MCFBuild.hpp" -o "%TempDir%\MCFBuild.hpp.gch" || exit /b 1
echo #warning Failed to load precompiled header file. > %TempDir%\MCFBuild.hpp
echo #include "../MCFBuild.hpp" >> %TempDir%\MCFBuild.hpp

for /f %%I in ('dir *.c *.cpp /b /o:-d /t:w') do (
	echo "%TempDir%\%%~nxI.o" ^<- "%%I"
	if "%%~xI"==".c" (
		start /b /belownormal gcc -c -o "%TempDir%\%%~nxI.o" %CFlags% "%%I" || exit /b 1
	) else if not "%%~nxI"=="MCFMain.cpp" (
		start /b /belownormal g++ -c -o "%TempDir%\%%~nxI.o" %CPPFlags% "%%I" -include "%TempDir%\MCFBuild.hpp" || exit /b 1
	)
)

:wait2
	sleep 1
	tasklist /fo csv | tail -n +2 | grep -P """cc1(plus)?.exe""" >nul 2>nul
	if not errorlevel 1 goto wait2

echo ----- Linking...
echo "%TempDir%\libmcfbuild.a" ^<- "%TempDir%\*.o"
ar rcus "%TempDir%\libmcfbuild.a" "%TempDir%\*.o"

echo "%ExeFile%" ^<- "MCFMain.cpp"
g++ -o "%ExeFile%" %CPPFlags% "MCFMain.cpp" -include "%TempDir%\MCFBuild.hpp" %LinkerFlags% %Libs% || exit /b 1

echo ----- Succeeded
