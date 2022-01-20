@echo off
setlocal
erase *.o *.lib
make lpeg.lib CC=icx RANLIB=
endlocal
