set OPTS=i
set arg1=%1
set arg2=%2
:: if arg3 is freeglut then freeglut not glut library will be built
set arg3=%3

@REM :: setup compiler environment
@REM if x%arg1% == xbot goto skip1
@REM call ..\..\..\Utilities\Scripts\setup_intel_compilers.bat
@REM :skip1

set EXIT_SCRIPT=1

set WAIT=
if "%arg1%"=="bot" (
  set WAIT=/WAIT
)

set LIBDIR=%CD%
@REM git clean -dxf

cd ..\..\..\Source
set SRCDIR=%CD%

cd ..\Build
set BUILDDIR=%CD%

echo "Making libs"

:: openvr
::cd %SRCDIR%\openvr
::call %WAIT% makelib %OPTS% -copy libopenvr.lib %LIBDIR%\openvr.lib

:: ZLIB
echo %SRCDIR%\zlib128
cd %SRCDIR%\zlib128
call makelib %OPTS% -copy libz.lib %LIBDIR%\zlib.lib

:: JPEG
cd %SRCDIR%\jpeg-9b
call makelib %OPTS% -copy libjpeg.lib  %LIBDIR%\jpeg.lib

:: PNG
cd %SRCDIR%\png-1.6.21
call makelib %OPTS% -copy libpng.lib %LIBDIR%\png.lib

:: GD
cd %SRCDIR%\gd-2.0.15
call makelib %OPTS% -copy libgd.lib %LIBDIR%\gd.lib

:: GLUT
if x%arg3% == xfreeglut goto skip_glut
cd %SRCDIR%\glut-3.7.6
call makelib %OPTS% -copy libglutwin.lib %LIBDIR%\glut32.lib
:skip_glut

:: GLUI
cd %SRCDIR%\glui_v2_1_beta
if x%arg3% == xfreeglut goto skip_glui1
  call makelib %OPTS% -copy libglui.lib %LIBDIR%\glui.lib
:skip_glui1

if NOT x%arg3% == xfreeglut goto skip_glui2
  call makelib_freeglut %OPTS% -copy libglui.lib %LIBDIR%\glui.lib
:skip_glui2

:: pthreads
cd %SRCDIR%\pthreads
call makelib %OPTS% -copy libpthreads.lib %LIBDIR%\pthreads.lib

if NOT x%arg2% == xlua goto skip_lua
:: Lua interpreter
cd %SRCDIR%\lua-5.3.1
call makelib.bat
copy src\liblua.lib %LIBDIR%\liblua.lib

:: LPEG
cd %SRCDIR%\lpeg-1.0.0
call makelib.bat
copy lpeg.lib %LIBDIR%\lpeg.lib
:skip_lua

:: FREEGLUT
if NOT "x%arg3%" == "xfreeglut" goto skip_freeglut
cd %BUILDDIR%\freeglut3.0.0\intel_win_64
call make_freeglut %OPTS%
copy freeglut_staticd.lib %LIBDIR%\freeglut_staticd.lib
copy freeglut_staticd.lib %LIBDIR%\glut32.lib
:skip_freeglut

cd %LIBDIR%

if "x%arg1%" == "xbot" goto skip1
pause
dir *.lib
pause
:skip1
