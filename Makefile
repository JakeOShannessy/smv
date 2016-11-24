
.PHONY: smv test clean

BUILD_PLATFORM=gnu_linux_64
BUILD_TARGET=$(BUILD_PLATFORM)$(DEBUG_SUFFIX)
SMV_BUILD_DIR = Build/smokeview/$(BUILD_PLATFORM)

smv:
	cd $(SMV_BUILD_DIR) && $(MAKE) -j 4 ${SMV_MAKE_OPTS} LUA_SCRIPTING=${LUA_SCRIPTING} BUILD_PLATFORM=$(BUILD_PLATFORM) BUILD_TARGET=$(BUILD_TARGET) -f ../Makefile.alternative.make

clean:
	cd $(SMV_BUILD_DIR) && $(MAKE) BUILD_PLATFORM=$(BUILD_PLATFORM) BUILD_TARGET=$(BUILD_TARGET) -f ../Makefile.alternative.make clean

test: smv
	cd Verification/Scripting && ./tests.sh
