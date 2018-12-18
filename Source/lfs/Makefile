T= lfs

CONFIG= ./config
EXT=.dll

include $(CONFIG)

SRCS= src/$T.c
OBJS= src/$T.o

lib: src/lfs.so

src/lfs.so: $(OBJS)
	MACOSX_DEPLOYMENT_TARGET="10.3"; export MACOSX_DEPLOYMENT_TARGET; $(CC) $(LIB_OPTION) -L$(LUA_LIBDIR) -o lfs$(EXT) $(OBJS) -llua53

test: lib
	LUA_CPATH=./src/?.so lua tests/test.lua

install:
	mkdir -p $(LUA_LIBDIR)
	cp src/lfs.so $(LUA_LIBDIR)

clean:
	rm -f src/lfs.so $(OBJS)
