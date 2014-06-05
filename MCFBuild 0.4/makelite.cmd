@echo off

echo ----- Cleaning up...
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
