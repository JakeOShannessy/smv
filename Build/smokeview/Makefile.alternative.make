
# Smokeview makefile

#To use this Makefile cd to a sub-directory and type make_smv.sh or make_smv.bat

SOURCE_DIR = ../../../Source/
BUILD_DIR = ../../../Build
LIB_DIR = $(BUILD_DIR)/LIBS

# Set the C standard we are using. Currently this is just the appropriate flag
# for gcc and is not applied to other compilers.
C_STANDARD = -std=gnu89

SMV_TESTFLAG =
SMV_TESTSTRING =
SMV_PROFILEFLAG =
SMV_PROFILESTRING =
COMP_VERSION =
LUA_SCRIPTING =
QUARTZ =

ifeq ($(shell echo "check_quotes"),"check_quotes")
  GIT_HASH := $(shell ..\..\..\Utilities\Scripts\githash)
  GIT_DATE := $(shell ..\..\..\Utilities\Scripts\gitlog)
else
  GIT_HASH := $(shell git describe --long --dirty)
  GIT_DATE := $(shell git log -1 --format=%cd)
endif
GITINFO=-Dpp_GITHASH=\"$(GIT_HASH)\" -Dpp_GITDATE=\""$(GIT_DATE)\""

ifeq ($(shell echo "check_quotes"),"check_quotes")
  INTEL_COMPVERSION := $(shell ..\..\..\Utilities\Scripts\intel_compversion)
  GNU_COMPVERSION := $(shell ..\..\..\Utilities\Scripts\gnu_compversion)
else
  INTEL_COMPVERSION := $(shell ../../../Utilities/Scripts/intel_compversion.sh)
  GNU_COMPVERSION := $(shell ../../../Utilities/Scripts/gnu_compversion.sh)
endif
INTEL_COMPINFO=-Dpp_COMPVER=\"$(INTEL_COMPVERSION)\"
GNU_COMPINFO=-Dpp_COMPVER=\"$(GNU_COMPVERSION)\"

#----------------------------------------------------------------------------
# Should not need to edit lines below except to add or 'debug' target entries

# The variable VPATH defines the source code directory relative to the current directory

VPATH = $(SOURCE_DIR)/smokeview:$(SOURCE_DIR)/shared:$(SOURCE_DIR)/glew
bin = .

# Definition of the object variables

obj = main.o menus.o IOscript.o IOshooter.o glui_objects.o glui_shooter.o glui_wui.o\
      csphere.o colortimebar.o smv_endian.o camera.o IOgeometry.o IOwui.o\
      IOobjects.o IOtour.o glui_display.o glui_tour.o getdatacolors.o smokeview.o\
      output.o renderfile.o isobox.o getdatabounds.o readsmv.o scontour2d.o\
      glui_smoke.o glui_clip.o glui_stereo.o glui_geometry.o glui_motion.o\
      glui_bounds.o dmalloc.o interp.o assert.o \
      compress.o IOvolsmoke.o IOsmoke.o IOplot3d.o IOslice.o IOboundary.o\
      IOpart.o IOzone.o IOiso.o callbacks.o drawGeometry.o\
      glui_colorbar.o skybox.o file_util.o string_util.o startup.o glui_trainer.o\
      shaders.o unit.o threader.o histogram.o translate.o update.o viewports.o\
      smv_geometry.o showscene.o glew.o infoheader.o  md5.o sha1.o sha256.o\
      fdsmodules.o gsmv.o getdata.o

ifeq ($(LUA_SCRIPTING),true)
obj += c_api.o lua_api.o
endif

objwin = $(obj:.o=.obj)

ifeq ($(QUARTZ),true)
INC = -I /usr/X11/include
else
INC = -I $(SOURCE_DIR)/glut-3.7.6
endif

INC += -I $(SOURCE_DIR)/glui_v2_1_beta -I $(SOURCE_DIR)/gd-2.0.15 -I $(SOURCE_DIR)/shared -I $(SOURCE_DIR)/smokeview -I $(SOURCE_DIR)/glew -I $(SOURCE_DIR)/zlib128
WININC = -I $(SOURCE_DIR)/GLINC -I $(SOURCE_DIR)/pthreads

ifeq ($(LUA_SCRIPTING),true)
INC += -I $(SOURCE_DIR)/lua-5.3.1/src
endif

SMVLUACORE_DIR = $(SOURCE_DIR)/smvluacore
SMVLUACORE_FILES = $(SMVLUACORE_DIR)/smv.lua $(SMVLUACORE_DIR)/ssf.lua \
	$(SMVLUACORE_DIR)/ssfparser.lua $(SMVLUACORE_DIR)/ssfcommands.lua \
	$(SMVLUACORE_DIR)/clipping.lua $(SMVLUACORE_DIR)/bounds.lua \
	$(SMVLUACORE_DIR)/render.lua $(SMVLUACORE_DIR)/load.lua \
	$(SMVLUACORE_DIR)/view.lua $(SMVLUACORE_DIR)/tour.lua \
	$(SMVLUACORE_DIR)/iniparser.lua $(SMVLUACORE_DIR)/inioptions.lua \
	$(SMVLUACORE_DIR)/unload.lua $(SMVLUACORE_DIR)/constants.lua \
	$(SMVLUACORE_DIR)/camera.lua $(SMVLUACORE_DIR)/window.lua

smvluacore: $(SMVLUACORE_FILES)
	cp $(SMVLUACORE_FILES) .
# cannot use $^ here

# ------------- libraries used by smokeview -------------------

#--- windows

WIN32_LIBS = user32.lib gdi32.lib comdlg32.lib shell32.lib
SMV_WIN_LIBDIR = $(LIB_DIR)/intel_win_64
SMV_LIBS_WIN = $(SMV_WIN_LIBDIR)\glui.lib $(SMV_WIN_LIBDIR)\glut32.lib $(SMV_WIN_LIBDIR)\gd.lib $(SMV_WIN_LIBDIR)\jpeg.lib $(SMV_WIN_LIBDIR)\png.lib $(SMV_WIN_LIBDIR)\zlib.lib $(SMV_WIN_LIBDIR)\pthreads.lib

#--- OSX

SMV_LIBS_OSX = -lglui -lgd -ljpeg -lpng -lz
INTEL_LIBS_OSX = $(IFORT_COMPILER_LIB)/libifcoremt.a $(IFORT_COMPILER_LIB)/libifport.a

#--- Linux

SMV_LIBS_LINUX = $(SMV_LIBS_OSX) -lglut
SYSTEM_LIBS_LINUX = -lpthread -lX11 -lXmu -lGLU -lGL -lm
INTEL_LIBS_LINUX =$(IFORT_COMPILER_LIB)/libifcore.a $(IFORT_COMPILER_LIB)/libifport.a

#*** General Purpose Rules ***

no_target:
	@echo \******** You did not specify a make target \********
	@echo Use one of targets found in the Makefile

.SUFFIXES: .F90 .c .obj .o .f90 .cpp

.c.obj:
	$(CC) -c $(CFLAGS) $(INC) $<
.c.o:
	$(CC) -c $(CFLAGS) $(C_STANDARD) $(INC) $<

.cpp.obj:
	$(CPP) -c $(CFLAGS) $(INC) $<
.cpp.o:
	$(CPP) -c $(CFLAGS) $(INC) $<

.F90.o:
	$(FC) -c $(FFLAGS) $<
.f90.obj:
	$(FC) -c $(FFLAGS) $<
.f90.o:
	$(FC) -c $(FFLAGS) $<

# ********  rules for each platform supported *************

# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV  Windows VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV

# note: use /fpscomp:general option to allow smokeview to read files being currently written to by FDS
#       (instead of the non-standard SHARED keyword in a FORTRAN OPEN statement)

# ------------- intel_win_64_db ----------------

intel_win_64_db : INC += $(WININC) -I $(SOURCE_DIR)/shared -I $(SOURCE_DIR)/smokeview
intel_win_64_db : FFLAGS    = /Od /iface:stdref /fpp -D WIN32 -D pp_INTEL /Zi /nologo /fpscomp:general /debug:full /extend_source:132 /warn:unused /warn:nointerfaces /Qtrapuv /fp:strict /fp:except /traceback /check:all /stand:f08
intel_win_64_db : CFLAGS    = /Od /W4 /debug:full /Zi -D _DEBUG -D WIN32 -D pp_INTEL -D _CONSOLE -D X64 -D GLEW_STATIC -D PTW32_STATIC_LIB $(SMV_TESTFLAG) $(GITINFO) $(INTEL_COMPINFO)
intel_win_64_db : LFLAGS    = /F32768000 /traceback /Zi /DEBUG
intel_win_64_db : CC        = icl
intel_win_64_db : CPP       = icl
intel_win_64_db : FC        = ifort
intel_win_64_db : exe       = smokeview_win_$(SMV_TESTSTRING)64_db

intel_win_64_db : $(objwin)
	$(CPP) -o $(bin)/$(exe) $(LFLAGS) $(objwin) $(WIN32_LIBS) $(SMV_LIBS_WIN)

# ------------- intel_win_64 ----------------

intel_win_64 : INC += $(WININC) -I $(SOURCE_DIR)/shared -I $(SOURCE_DIR)/smokeview
ifeq ($(LUA_SCRIPTING),true)
intel_win_64 : INC += -I $(SOURCE_DIR)/lua-5.3.1/src
endif
intel_win_64 : FFLAGS    = -O2 /iface:stdref /fpp -D WIN32 -D pp_INTEL  /fpscomp:general
intel_win_64 : CFLAGS    = -O1 -D WIN32 -D pp_INTEL -D _CONSOLE -D X64 -D GLEW_STATIC -D PTW32_STATIC_LIB $(SMV_TESTFLAG) $(GITINFO) $(INTEL_COMPINFO)
intel_win_64 : LIBLUA    =
ifeq ($(LUA_SCRIPTING),true)
intel_win_64 : CFLAGS    += -D pp_LUA
intel_win_64 : SMV_LIBS_WIN += $(WIN_LIBDIR)\liblua.lib
intel_win_64 : SMV_LIBS_WIN += $(WIN_LIBDIR)\lpeg.lib
intel_win_64 : SMVLUACORE_FILES += $(WIN_LIBDIR)/lpeg.dll
endif
intel_win_64 : LFLAGS    = /F32768000
intel_win_64 : CC        = icl
intel_win_64 : CPP       = icl
intel_win_64 : FC        = ifort
intel_win_64 : exe       = smokeview_win_$(SMV_TESTSTRING)64

intel_win_64 : $(objwin)
	$(CPP) -o $(bin)/$(exe) $(LFLAGS) $(objwin) $(WIN32_LIBS) $(SMV_LIBS_WIN) $(LIBLUA)

# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV  Linux VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV

# to profile smokeview on linux
# 1.  build  using ./make_smv.sh -p
# 2.  run case as usual
# 3.  type: gprof smokeview_application_name > results.out

# ------------- intel_linux_64 ----------------

intel_linux_64 : LIB_DIR_PLAT = $(LIB_DIR)/intel_linux_64
intel_linux_64 : LIBS_PLAT =
intel_linux_64 : FFLAGS    = -O0 -traceback -m64 -static-intel -D pp_INTEL  -fpp $(SMV_PROFILEFLAG)
intel_linux_64 : CFLAGS    = -O0 -traceback -m64 -static-intel -D pp_LINUX -D pp_INTEL $(SMV_TESTFLAG) $(SMV_PROFILEFLAG) $(GITINFO) $(INTEL_COMPINFO)
intel_linux_64 : LIBLUA    =
ifeq ($(LUA_SCRIPTING),true)
intel_linux_64 : CFLAGS    += -D pp_LUA
intel_linux_64 : LIBS_PLAT += $(LIB_DIR_PLAT)/lua53.a
intel_linux_64 : SMVLUACORE_FILES += $(LIB_DIR_PLAT)/lpeg.so
intel_linux_64 : LIBLUA    += $(LIB_DIR_PLAT)/liblua.a -ldl
endif
intel_linux_64 : LFLAGS    = -m64 -static-intel $(INTEL_LIBS_LINUX) $(SMV_PROFILEFLAG)
intel_linux_64 : CC        = icc
intel_linux_64 : CPP       = icpc
intel_linux_64 : FC        = ifort
intel_linux_64 : exe       = smokeview_linux_$(SMV_TESTSTRING)64$(SMV_PROFILESTRING)

intel_linux_64 : $(obj)  $(if $(LUA_SCRIPTING),smvluacore)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS) -L$(LIB_DIR_PLAT) $(SMV_LIBS_LINUX) \
        $(INTEL_LIBS_LINUX)\
        $(SYSTEM_LIBS_LINUX) $(LIBLUA)

# ------------- intel_linux_64_db ----------------

intel_linux_64_db : FFLAGS    = -O0 -m64 -static-intel -traceback -g -fpe0 -fltconsistency -D pp_INTEL  -WB -fpp -stand:f08 $(SMV_PROFILEFLAG)
intel_linux_64_db : CFLAGS    = -O0 -g -m64 -static-intel $(SMV_TESTFLAG) -D _DEBUG -D pp_LINUX -D pp_INTEL $(SMV_PROFILEFLAG) -traceback -Wall -Wextra -check=stack,uninit -fp-stack-check -fp-trap-all=divzero,invalid,overflow -ftrapuv -Wuninitialized -Wunused-function -Wunused-variable $(GITINFO) $(INTEL_COMPINFO)
intel_linux_64_db : LFLAGS    = -m64 -static-intel $(INTEL_LIBS_LINUX) -traceback $(SMV_PROFILEFLAG)
intel_linux_64_db : CC        = icc
intel_linux_64_db : CPP       = icpc
intel_linux_64_db : FC        = ifort
intel_linux_64_db : exe       = smokeview_linux_$(SMV_TESTSTRING)64_db$(SMV_PROFILESTRING)

intel_linux_64_db : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS) -L$(LIB_DIR)/intel_linux_64 $(SMV_LIBS_LINUX) $(INTEL_LIBS_LINUX) $(SYSTEM_LIBS_LINUX)

# ------------- gnu_linux_64_db ----------------

gnu_linux_64_db : FFLAGS    = -O0 -m64 -ggdb -Wall -x f95-cpp-input -D pp_GCC -ffree-form -frecord-marker=4 -fcheck=all -fbacktrace
gnu_linux_64_db : CFLAGS    = -O0 -m64 -ggdb -Wall -Wno-parentheses -Wno-unknown-pragmas -Wno-comment -Wno-write-strings -D _DEBUG -D pp_LINUX -D pp_GCC $(SMV_TESTFLAG) $(GNU_COMPINFO) $(GITINFO)
gnu_linux_64_db : LFLAGS    = -m64
gnu_linux_64_db : CC        = gcc
gnu_linux_64_db : CPP       = g++
gnu_linux_64_db : FC        = gfortran
gnu_linux_64_db : exe       = smokeview_linux_$(SMV_TESTSTRING)64_db

gnu_linux_64_db : LIB_DIR_PLAT = $(LIB_DIR)/gnu_linux_64
gnu_linux_64_db : LIBS_PLAT = $(LIB_DIR_PLAT)/libglui.a \
	$(LIB_DIR_PLAT)/libgd.a $(LIB_DIR_PLAT)/libjpeg.a \
	$(LIB_DIR_PLAT)/libpng.a $(LIB_DIR_PLAT)/libz.a \
	$(LIB_DIR_PLAT)/libglut.a

ifeq ($(LUA_SCRIPTING),true)
gnu_linux_64_db : CFLAGS    += -D pp_LUA
gnu_linux_64_db : SMVLUACORE_FILES += $(LIB_DIR_PLAT)/lpeg.so
gnu_linux_64_db : LIBS_PLAT += $(LIB_DIR_PLAT)/liblua.a $(LIB_DIR_PLAT)/lpeg.so
endif

gnu_linux_64_db : $(exe) $(if $(LUA_SCRIPTING),$(SMVLUACORE_FILES))

$(exe) : $(obj) $(LIBS_PLAT)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS) -L $(LIB_DIR_PLAT) \
		$(SMV_LIBS_LINUX) -lgfortran $(SYSTEM_LIBS_LINUX) \
		$(if $(LUA_SCRIPTING),$(LIB_DIR_PLAT)/liblua.a -ldl)

$(LIBS_PLAT):
	@echo Making lib: $(notdir $@)
	cd $(LIB_DIR_PLAT) && $(MAKE) -f ../Makefile $(notdir $@)

# ------------- gnu_linux_64 ----------------


gnu_linux_64 : LIB_DIR_PLAT = $(LIB_DIR)/gnu_linux_64
gnu_linux_64 : LIBS_PLAT = $(LIB_DIR_PLAT)/libglui.a \
	$(LIB_DIR_PLAT)/libgd.a $(LIB_DIR_PLAT)/libjpeg.a \
	$(LIB_DIR_PLAT)/libpng.a $(LIB_DIR_PLAT)/libz.a \
	$(LIB_DIR_PLAT)/libglut.a
gnu_linux_64 : FFLAGS    = -O0 -m64 -x f95-cpp-input -D pp_GCC -ffree-form -frecord-marker=4
gnu_linux_64 : CFLAGS    = -O0 -m64 -D pp_LINUX -D pp_GCC $(SMV_TESTFLAG) -Wno-write-strings $(GNU_COMPINFO) $(GITINFO)
ifeq ($(LUA_SCRIPTING),true)
gnu_linux_64 : CFLAGS    += -D pp_LUA
gnu_linux_64 : LIBS_PLAT += $(LIB_DIR_PLAT)/liblua.a $(LIB_DIR_PLAT)/lpeg.so
gnu_linux_64 : SMVLUACORE_FILES += $(LIB_DIR_PLAT)/lpeg.so
endif
gnu_linux_64 : LFLAGS    = -m64
gnu_linux_64 : CC        = gcc
gnu_linux_64 : CPP       = g++
gnu_linux_64 : FC        = gfortran
gnu_linux_64 : exe       = smokeview_linux_$(SMV_TESTSTRING)64

gnu_linux_64 : $(exe) $(if $(LUA_SCRIPTING),$(SMVLUACORE_FILES))

$(exe) : $(obj) $(LIBS_PLAT)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS) -L $(LIB_DIR_PLAT) \
		$(SMV_LIBS_LINUX) -lgfortran $(SYSTEM_LIBS_LINUX) \
		$(if $(LUA_SCRIPTING),$(LIB_DIR_PLAT)/liblua.a -ldl)

$(LIBS_PLAT):
	@echo Making lib: $(notdir $@)
	cd $(LIB_DIR_PLAT) && $(MAKE) -f ../Makefile $(notdir $@)

# ------------- mingw_win_64 ----------------

mingw_win_64 : LIB_DIR_PLAT = $(LIB_DIR)/mingw_win_64
mingw_win_64 : LIBS_PLAT = $(LIB_DIR_PLAT)/libglui.a \
	$(LIB_DIR_PLAT)/libgd.a $(LIB_DIR_PLAT)/libjpeg.a \
	$(LIB_DIR_PLAT)/libpng.a $(LIB_DIR_PLAT)/libz.a \
	$(LIB_DIR_PLAT)/libglutwin.a
mingw_win_64 : INC += -I $(SOURCE_DIR)/GLINC-mingw -I $(SOURCE_DIR)/pthreads
mingw_win_64 : FFLAGS    = -O0 -m64 -x f95-cpp-input -D pp_GCC \
						       -ffree-form -frecord-marker=4
mingw_win_64 : CFLAGS    = -O0 -m64 -D pp_LINUX -D GLEW_STATIC -D MINGW
ifeq ($(LUA_SCRIPTING),true)
mingw_win_64 : CFLAGS    += -D pp_LUA
mingw_win_64 : LIBS_PLAT += $(LIB_DIR_PLAT)/lua53.dll
mingw_win_64 : SMVLUACORE_FILES += $(LIB_DIR_PLAT)/lpeg.dll
mingw_win_64 : LIBS_PLAT += $(LIB_DIR_PLAT)/lua53.dll $(LIB_DIR_PLAT)/lpeg.dll
endif
mingw_win_64 : LFLAGS    = -m64 -static-libgcc -static-libstdc++ -lgcov
mingw_win_64 : CC        = gcc
mingw_win_64 : CPP       = g++
mingw_win_64 : FC        = gfortran
mingw_win_64 : exe       = smokeview_mingw_$(SMV_TESTSTRING)64.exe
mingw_win_64 : execn     = $(bin)/$(exe)

mingw_win_64 : excn = smokeview_mingw_64.exe

mingw_win_64 : $(excn) $(if $(LUA_SCRIPTING),$(SMVLUACORE_FILES))
	@echo $(GITINFO)
	@echo $^

$(excn): $(obj) $(LIBS_PLAT)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS) -L $(LIB_DIR_PLAT) \
		$(($(LUA_SCRIPTING),true),-I $(SOURCE_DIR)/lua-5.3.1/src) \
		$(LIBS_PLAT) -lgfortran -lm -lopengl32 -lglu32 \
		-lgdi32 -lwinmm -lcomdlg32 -lpthread

$(LIBS_PLAT):
	@echo Making lib: $(notdir $@)
	cd $(LIB_DIR_PLAT) && $(MAKE) -f ../Makefile $(notdir $@)
# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV   OSX   VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
# VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV

OSX_INTEL_LIBS = $(IFORT_COMPILER_LIB)/libifcoremt.a $(IFORT_COMPILER_LIB)/libimf.a $(IFORT_COMPILER_LIB)/libirc.a $(IFORT_COMPILER_LIB)/libsvml.a

# ------------- intel_osx_64 ----------------

intel_osx_64 : LIB_DIR_PLAT = $(LIB_DIR)/intel_osx_64
intel_osx_64 : LIBS_PLAT =
intel_osx_64 : FFLAGS    = -O0 -m64 -static-intel -fpp -D pp_OSX -mmacosx-version-min=10.4
intel_osx_64 : CFLAGS    = -O0 -m64 -static-intel -D pp_OSX  -D pp_INTEL -mmacosx-version-min=10.4 $(SMV_TESTFLAG) $(SMV_PROFILEFLAG) $(GITINFO)
intel_osx_64 : LIBLUA    =
ifeq ($(LUA_SCRIPTING),true)
intel_osx_64 : CFLAGS    += -D pp_LUA
intel_osx_64 : LIBS_PLAT += $(LIB_DIR_PLAT)/lua53.a
intel_osx_64 : SMVLUACORE_FILES += $(LIB_DIR_PLAT)/lpeg.so
intel_osx_64 : LIBLUA    += $(LIB_DIR_PLAT)/liblua.a -ldl
endif
intel_osx_64 : LFLAGS    = -m64 -static-intel -framework OpenGL -framework GLUT -mmacosx-version-min=10.4
intel_osx_64 : CC        = icc
intel_osx_64 : CPP       = icpc
intel_osx_64 : FC        = ifort
intel_osx_64 : exe       = smokeview_osx_$(SMV_TESTSTRING)64 $(SMV_PROFILESTRING)

intel_osx_64 : $(obj) $(if $(LUA_SCRIPTING),smvluacore)
	icpc -o $(bin)/$(exe) $(LFLAGS) $(obj)  -L $(LIB_DIR_PLAT) $(SMV_LIBS_OSX) \
        $(INTEL_LIBS_OSX) $(LIBLUA)

# ------------- intel_osx_64q ----------------

intel_osx_64q : LIB_DIR_PLAT = $(LIB_DIR)/intel_osx_64
intel_osx_64q : LIBS_PLAT =
intel_osx_64q : FFLAGS    = -O0 -m64 -static-intel -fpp -D pp_INTEL -D pp_OSX -mmacosx-version-min=10.7
intel_osx_64q : CFLAGS    = -O0 -m64 -static-intel -I /usr/X11/include -D pp_QUARTZ -D pp_OSX -D pp_INTEL -mmacosx-version-min=10.7 $(SMV_TESTFLAG) $(GITINFO) $(INTEL_COMPINFO)
intel_osx_64q : LIBLUA    =
ifeq ($(LUA_SCRIPTING),true)
intel_osx_64q : CFLAGS    += -D pp_LUA
intel_osx_64q : LIBS_PLAT += $(LIB_DIR_PLAT)/lua53.a
intel_osx_64q : SMVLUACORE_FILES += $(LIB_DIR_PLAT)/lpeg.so
intel_osx_64q : LIBLUA    += $(LIB_DIR_PLAT)/liblua.a -ldl
endif
intel_osx_64q : LFLAGS    = -m64 -static-intel -L/usr/X11/lib -lX11 -lXmu -lGLU -lGL -mmacosx-version-min=10.7
intel_osx_64q : CC        = icc
intel_osx_64q : CPP       = icpc
intel_osx_64q : FC        = ifort
intel_osx_64q : exe       = smokeview_osx_$(SMV_TESTSTRING)64q

intel_osx_64q : $(obj) $(if $(LUA_SCRIPTING),smvluacore)
	icpc -o $(bin)/$(exe) $(LFLAGS) $(obj)  -L $(LIB_DIR_PLAT) $(SMV_LIBS_OSX) -lglut \
        $(INTEL_LIBS_OSX) $(LIBLUA)

# ------------- intel_osx_64_db ----------------

intel_osx_64_db : FFLAGS    = -O0 -g -m64 -traceback -static-intel -fpp      -D pp_OSX -D pp_INTEL -mmacosx-version-min=10.7 -stand:f08
intel_osx_64_db : CFLAGS    = -O0 -g -m64 -traceback -static-intel -D _DEBUG -D pp_OSX -D pp_INTEL -mmacosx-version-min=10.7 $(SMV_TESTFLAG) -Wall -Wextra -check=stack,uninit -fp-stack-check -fp-trap-all=divzero,invalid,overflow -ftrapuv -Wuninitialized -Wunused-function -Wunused-variable $(GITINFO) $(INTEL_COMPINFO)
intel_osx_64_db : LFLAGS    = -m64 -traceback -static-intel -framework OpenGL -framework GLUT -mmacosx-version-min=10.7
intel_osx_64_db : CC        = icc
intel_osx_64_db : CPP       = icpc
intel_osx_64_db : FC        = ifort
intel_osx_64_db : exe       = smokeview_osx_$(SMV_TESTSTRING)64_db

intel_osx_64_db : $(obj)
	icpc -o $(bin)/$(exe) $(LFLAGS) $(obj)  -L $(LIB_DIR)/intel_osx_64 $(SMV_LIBS_OSX) $(INTEL_LIBS_OSX)

# ------------- gnu_osx_64_db ----------------

gnu_osx_64_db : FFLAGS    = -O0 -m64 -x f95-cpp-input -D pp_GCC -ffree-form
gnu_osx_64_db : CFLAGS    = -O0 -m64 -D _DEBUG -D pp_OSX -D pp_GCC $(SMV_TESTFLAG) -Wall -Wno-deprecated-declarations -Wno-write-strings $(GNU_COMPINFO) $(GITINFO)
gnu_osx_64_db : LFLAGS    = -m64 -framework OpenGL -framework GLUT
gnu_osx_64_db : CC        = gcc
gnu_osx_64_db : CPP       = g++
gnu_osx_64_db : FC        = gfortran
gnu_osx_64_db : exe       = smokeview_osx_64_db

gnu_osx_64_db : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS) -L$(LIB_DIR)/gnu_osx_64 $(SMV_LIBS_OSX) -L $(GLIBDIR) -lgfortran

# ------------- gnu_osx_64 ----------------

gnu_osx_64 : FFLAGS    = -O0 -m64 -x f95-cpp-input -D pp_GCC -ffree-form
gnu_osx_64 : CFLAGS    = -O0 -m64 -D pp_OSX -D pp_GCC $(SMV_TESTFLAG) -Wno-write-strings $(GNU_COMPINFO) $(GITINFO)
gnu_osx_64 : LFLAGS    = -m64 -framework OpenGL -framework GLUT
gnu_osx_64 : CC        = gcc
gnu_osx_64 : CPP       = g++
gnu_osx_64 : FC        = gfortran
gnu_osx_64 : exe       = smokeview_osx_64

gnu_osx_64 : $(obj)
	$(CPP) -o $(bin)/$(exe) $(obj) $(LFLAGS) -L$(LIB_DIR)/gnu_osx_64 $(SMV_LIBS_OSX) -L $(GLIBDIR) -lgfortran

#-------------- Clean Target to remove Object and Module files -----------

.PHONY : clean
clean:
	-rm -f *.o *.mod *.lua *.dll *.so *.exe
	-rm -f *.gcno *.gcda
	-rm -f $(LIBS_PLAT)
	cd $(LIB_DIR_PLAT) && $(MAKE) -f ../Makefile $@

.FORCE:

# *** Object Dependencies ***

gsmv.o : fdsmodules.o
getdata.o : gsmv.o fdsmodules.o

gsmv.obj : fdsmodules.obj
getdata.obj : gsmv.obj fdsmodules.obj
