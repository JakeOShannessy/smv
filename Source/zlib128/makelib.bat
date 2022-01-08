@REM @echo off
@REM setlocal
@REM call ..\scripts\setopts %*
@REM title Building windows zlib library
@REM erase *.o *.obj libz.a libz.lib
@REM set target=libz.lib
@REM if %COMPILER% == gcc set target=libz.a
@REM make COMPILER=%COMPILER% SIZE=%SIZE% -f ./makefile %target%
@REM if %COPYLIB% == 1 copy %FROMLIB% %TOLIB%
@REM if "x%EXIT_SCRIPT%" == "x" goto skip1
@REM exit
@REM :skip1
@REM endlocal
icx -c -O2 adler32.c