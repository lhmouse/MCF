@echo off

set Config=Debug

if "%1"=="Release" (
	set Config=Release
)

set TempDir=.Temp-%Config%

if not exist "%TempDir%\libmcflite.a" call clean %* || exit /b 1

echo ----- Compiling MCFBuild...
echo MCFBuild.hpp
g++ %CPPFlags% "MCFBuild.hpp" -o "%TempDir%\MCFBuild.hpp.gch" || exit /b 1
echo #warning Failed to load precompiled header file. > %TempDir%\MCFBuild.hpp
echo #include "../MCFBuild.hpp" >> %TempDir%\MCFBuild.hpp

for /f %%I in ('dir *.c *.cpp /b /o:-d /t:w') do (
	echo %%I
	if "%%~xI"==".c" (
		gcc %CFlags% "%%I" -c -o "%TempDir%\%%~nxI.o" || exit /b 1
	) else (
		g++ %CPPFlags% "%%I" -include "%TempDir%\MCFBuild.hpp" -c -o "%TempDir%\%%~nxI.o" || exit /b 1
	)
)

echo ----- Linking...
echo   =^> %ExeFile%
g++ "%TempDir%\*.o" %LinkerFlags% -o "%ExeFile%" -L"%TempDir%" -lmcflite -lstdc++ -lgcc -lgcc_eh -lmingwex -lmcflite || exit /b 1

echo ----- Succeeded
