OPTS="-g -6"
LIBDIR=.
SRCDIR=$(LIBDIR)/../../../Source

.PHONY: clean all no_target

no_target:
	@echo \******** You did not specify a make target \********
	@echo Use one of targets found in the Makefile

all: libgd.a libglui.a libglutwin.a libjpeg.a libpng.a libz.a libpthreads.a \
	liblua.a lua53.dll lpeg.dll lfs.dll

# GD
libgd.a: $(SRCDIR)/gd-2.0.15/libgd.a
	cp $^ .

$(SRCDIR)/gd-2.0.15/libgd.a:
	cd $(SRCDIR)/gd-2.0.15 && make libgd.a COMPILER=gcc COMPILER2=g++ SIZE="-m64"
#
# cd "$SRCDIR"/gd-2.0.15
# ./makelib.sh $OPTS
# cp libgd.a "$LIBDIR"/.
#
# GLUI
GLUI_DIR=$(SRCDIR)/glui_v2_1_beta
GLUI_LIB=$(GLUI_DIR)/libglui.a
libglui.a: $(GLUI_LIB)
	cp $^ .
$(GLUI_LIB):
	cd $(GLUI_DIR) && make COMPILER=gcc COMPILER2=g++ SIZE="-m64" # CFLAGOPT=${CFLAGOPT} SIZE=${SIZE} PLATFORM=\"${PLATFORM}\"
# cd "$SRCDIR"/glui_v2_1_beta
# ./makelib.sh $OPTS
# cp libglui.a "$LIBDIR"/.
#
# GLUT
GLUT_DIR=$(SRCDIR)/glut-3.7.6
GLUT_LIB_WIN=$(GLUT_DIR)/libglutwin.a
libglutwin.a: $(GLUT_LIB_WIN)
	cp $^ .
$(GLUT_LIB_WIN):
	cd $(GLUT_DIR) && make libglutwin.a COMPILER=gcc COMPILER2=g++ SIZE="-m64" # CFLAGOPT=${CFLAGOPT} SIZE=${SIZE} PLATFORM=\"${PLATFORM}\"

GLUT_LIB=$(GLUT_DIR)/libglut.a
libglut.a: $(GLUT_LIB)
	cp $^ .
$(GLUT_LIB):
	cd $(GLUT_DIR) && make libglut.a COMPILER=gcc COMPILER2=g++ SIZE="-m64" # CFLAGOPT=${CFLAGOPT} SIZE=${SIZE} PLATFORM=\"${PLATFORM}\"
# cd "$SRCDIR"/glut-3.7.6
# export TARGET=libglutwin.a
# ./makelib.sh $OPTS
# cp libglutwin.a "$LIBDIR"/.
#
# # JPEG
JPEG_DIR=$(SRCDIR)/jpeg-9b
JPEG_LIB=$(JPEG_DIR)/libjpeg.a
libjpeg.a: $(JPEG_LIB)
	cp $^ .
$(JPEG_LIB):
	cd $(JPEG_DIR) && make libjpeg.a COMPILER=gcc COMPILER2=g++ SIZE="-m64" # CFLAGOPT=${CFLAGOPT} SIZE=${SIZE} PLATFORM=\"${PLATFORM}\"
# cd "$SRCDIR"/jpeg-6b
# ./makelib.sh $OPTS
# cp libjpeg.a "$LIBDIR"/.
#
# # PNG
PNG_DIR=$(SRCDIR)/png-1.6.21
PNG_LIB=$(PNG_DIR)/libpng.a
libpng.a: $(PNG_LIB)
	cp $^ .
$(PNG_LIB):
	cd $(PNG_DIR) && make libpng.a COMPILER=gcc COMPILER2=g++ SIZE="-m64" # CFLAGOPT=${CFLAGOPT} SIZE=${SIZE} PLATFORM=\"${PLATFORM}\"
# cd "$SRCDIR"/png125
# ./makelib.sh $OPTS
# cp libpng.a "$LIBDIR"/.
#
# # ZLIB
ZLIB_DIR=$(SRCDIR)/zlib128
ZLIB_LIB=$(ZLIB_DIR)/libz.a
libz.a: $(ZLIB_LIB)
	cp $^ .
$(ZLIB_LIB):
	cd $(ZLIB_DIR) && make libz.a COMPILER=gcc COMPILER2=g++ SIZE="-m64" # CFLAGOPT=${CFLAGOPT} SIZE=${SIZE} PLATFORM=\"${PLATFORM}\"
# cd "$SRCDIR"/zlib114
# ./makelib.sh $OPTS
# cp libz.a "$LIBDIR"/.
#
# # PTHREADS
PTHREADS_DIR=$(SRCDIR)/pthreads
PTHREADS_LIB=$(PTHREADS_DIR)/libpthreads.a
libpthreads.a: $(PTHREADS_LIB)
	cp $^ .
$(PTHREADS_LIB):
	cd $(PTHREADS_DIR) && make libpthreads.a COMPILER=gcc COMPILER2=g++ SIZE="-m64" # CFLAGOPT=${CFLAGOPT} SIZE=${SIZE} PLATFORM=\"${PLATFORM}\"
# cd "$SRCDIR"/pthreads
# ./makelib.sh $OPTS
# cp libpthreads.a "$LIBDIR"/.
# #
# # # LUA
ifeq ($(BUILD_PLATFORM),mingw_win_64)
LUA_PLAT=mingw
endif
ifeq ($(BUILD_PLATFORM),gnu_linux_64)
LUA_PLAT=linux
endif
LUA_DIR=$(SRCDIR)/lua-5.3.1
LUA_LIB_STATIC=$(LUA_DIR)/src/liblua.a
liblua.a: $(LUA_LIB_STATIC)
	cp $^ .
$(LUA_LIB_STATIC):
	cd $(LUA_DIR) && make $(LUA_PLAT) COMPILER=gcc COMPILER2=g++ SIZE="-m64" # CFLAGOPT=${CFLAGOPT} SIZE=${SIZE} PLATFORM=\"${PLATFORM}\"
# # cd "$SRCDIR"/lua-5.3.1
# # make mingw
# # cp src/liblua.a "$LIBDIR"/.
#
# # LUA
LUA_LIB_DYN=$(LUA_DIR)/src/lua53.dll
lua53.dll: $(LUA_LIB_DYN)
	cp $^ .
$(LUA_LIB_DYN):
	cd $(LUA_DIR) && make mingw COMPILER=gcc COMPILER2=g++ SIZE="-m64" # CFLAGOPT=${CFLAGOPT} SIZE=${SIZE} PLATFORM=\"${PLATFORM}\"

# cd "$SRCDIR"/lua-5.3.1
# make mingw
# cp src/lua53.dll "$LIBDIR"/.
#
# LPEG
LPEG_DIR=$(SRCDIR)/lpeg-1.0.0
LPEG_LIB=$(LPEG_DIR)/lpeg.dll
lpeg.dll: $(LPEG_LIB)
	cp $^ .
lpeg.so: $(LPEG_DIR)/lpeg.so
$(LPEG_LIB): lua53.dll
	cd $(LPEG_DIR) && make $(LUA_PLAT) COMPILER=gcc COMPILER2=g++ SIZE="-m64" # CFLAGOPT=${CFLAGOPT} SIZE=${SIZE} PLATFORM=\"${PLATFORM}\"
$(LPEG_DIR)/lpeg.so:
	cd $(LPEG_DIR) && make $(LUA_PLAT) COMPILER=gcc COMPILER2=g++ SIZE="-m64" # CFLAGOPT=${CFLAGOPT} SIZE=${SIZE} PLATFORM=\"${PLATFORM}\"

# cd "$SRCDIR"/lpeg-1.0.0
# make
# cp lpeg.dll "$LIBDIR"/../../mingw_64/lpeg.dll
# # cp lpeg.a "$LIBDIR"/liblpeg.a

# LFS
LFS_DIR=$(SRCDIR)/luafilesystem
LFS_LIB=$(LFS_DIR)/lfs.dll
lfs.dll: $(LFS_LIB)
	cp $^ .
$(LFS_LIB):
	cd $(LFS_DIR) && make COMPILER=gcc COMPILER2=g++ SIZE="-m64" # CFLAGOPT=${CFLAGOPT} SIZE=${SIZE} PLATFORM=\"${PLATFORM}\"

# these commented lines have no corresponding clean target
clean:
	# cd $(SRCDIR)/gd-2.0.15 && make $@
	# cd $(SRCDIR)/glui_v2_1_beta && make $@
	# cd $(SRCDIR)/glut-3.7.6 && make $@
	# cd $(SRCDIR)/jpeg-9b && make $@
	# cd $(SRCDIR)/png-1.6.21 && make $@
	# cd $(SRCDIR)/zlib128 && make $@
	# cd $(SRCDIR)/pthreads && make $@
	cd $(SRCDIR)/lua-5.3.1 && make $@
	cd $(SRCDIR)/lpeg-1.0.0 && make $@
	# cd $(SRCDIR)/luafilesystem && make $@
