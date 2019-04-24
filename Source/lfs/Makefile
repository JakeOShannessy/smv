PROJECT_DIRECTORY=luafilesystem
T= lfs

CONFIG= ./config
EXT=.dll

include $(CONFIG)

SRCS= $(PROJECT_DIRECTORY)/src/$T.c
OBJS= $(PROJECT_DIRECTORY)/src/$T.o

lib: $(PROJECT_DIRECTORY)/src/lfs$(EXT)

linux: EXT = .so
linux: lib

$(PROJECT_DIRECTORY)/src/lfs$(EXT): $(OBJS)
	MACOSX_DEPLOYMENT_TARGET="10.3"; export MACOSX_DEPLOYMENT_TARGET; $(CC) $(LIB_OPTION) -L$(LUA_LIBDIR) -o lfs$(EXT) $(OBJS)

test: lib
	LUA_CPATH=$(PROJECT_DIRECTORY)/src/?.so lua $(PROJECT_DIRECTORY)/tests/test.lua

install:
	mkdir -p $(LUA_LIBDIR)
	cp $(PROJECT_DIRECTORY)/src/lfs.so $(LUA_LIBDIR)

clean:
	rm -f $(PROJECT_DIRECTORY)/src/lfs.so $(OBJS)
