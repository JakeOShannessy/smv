
.PHONY: smv test clean

BUILD_PLATFORM=mingw
ifeq ($(BUILD_PLATFORM),mingw)
SMV_BUILD_DIR = Build/smokeview/mingw_win_64
endif
ifeq ($(BUILD_PLATFORM),gnu_linux)
SMV_BUILD_DIR = Build/smokeview/gnu_linux_64
endif

smv:
	cd $(SMV_BUILD_DIR) && $(MAKE) -j 4 ${SMV_MAKE_OPTS} LUA_SCRIPTING=${LUA_SCRIPTING} BUILD_PLATFORM=$(BUILD_PLATFORM) -f ../Makefile.alternative.make

clean:
	cd $(SMV_BUILD_DIR) && $(MAKE) BUILD_PLATFORM=$(BUILD_PLATFORM) -f ../Makefile.alternative.make clean

test: smv
	cd Verification/Scripting && ./tests.sh
