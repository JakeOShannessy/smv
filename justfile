set windows-powershell := true
alias b := build
alias br := build-release

_default:
    @just --list

# Run the tests
test:
    ctest --test-dir cbuild -C Debug

# Build the debug binaries
build-win:
    cmake -B cbuild -DCMAKE_BUILD_TYPE=Debug -DVCPKG_TARGET_TRIPLET=x64-windows -DVCPKG_HOST_TRIPLET=x64-windows -DCMAKE_TOOLCHAIN_FILE="../vcpkg/scripts/buildsystems/vcpkg.cmake" -DVENDORED_UI_LIBS=ON
    cmake --build cbuild --config Debug -j6
    cmake --install cbuild --config Debug --prefix dist-debug

# Build the release binaries
build-release-win:
    cmake -B cbuild -DCMAKE_BUILD_TYPE=Release -DVCPKG_TARGET_TRIPLET=x64-windows -DVCPKG_HOST_TRIPLET=x64-windows -DCMAKE_TOOLCHAIN_FILE="../vcpkg/scripts/buildsystems/vcpkg.cmake"
    cmake --build cbuild --config Release -v -j6
    cmake --install cbuild --config Release --prefix dist

package-cmake-win: build-release-win
     cpack --config ./cbuild/CPackConfig.cmake

# Build the debug binaries
build:
    cmake -B cbuild -DCMAKE_BUILD_TYPE=Debug -DVENDORED_UI_LIBS=ON
    cmake --build cbuild --config Debug -j6
    cmake --install cbuild --config Debug --prefix dist-debug

build-strict:
    cmake -B cbuild -DSTRICT_CHECKS=ON -DCMAKE_BUILD_TYPE=Debug -DVENDORED_UI_LIBS=ON
    cmake --build cbuild --config Debug -j6
    cmake --install cbuild --config Debug --prefix dist-debug

# Build the release binaries
build-release:
    cmake -B cbuild -DCMAKE_BUILD_TYPE=Release
    cmake --build cbuild --config Release -v -j6
    cmake --install cbuild --config Release --prefix dist

package-cmake: build-release
     cpack --config ./cbuild/CPackConfig.cmake


# Build the debug binaries with intel OneAPi
build-intel:
    cmd.exe "/K" '"C:\Program Files (x86)\Intel\oneAPI\setvars.bat" && powershell just build-intel-internal'

build-intel-internal:
    cmake -B cbuild -DCMAKE_BUILD_TYPE=Debug -DVCPKG_TARGET_TRIPLET=x64-windows \
        -DVCPKG_HOST_TRIPLET=x64-windows -DCMAKE_TOOLCHAIN_FILE="../vcpkg/scripts/buildsystems/vcpkg.cmake" \
        -D CMAKE_C_COMPILER=icx -D CMAKE_CXX_COMPILER=icx -GNinja
    cmake --build cbuild --config Debug -j6
    cmake --install cbuild --config Debug --prefix dist-debug

# Build release and create MSI package
package-windows: build-release
    candle "SMVLuaInstaller.wxs"
    light "SMVLuaInstaller.wixobj" -ext WixUIExtension

# Build release and create RPM package
package-rpm:
    ./buildrpm.sh

# Clean everything
clean:
    cmake -E rm -rf cbuild  _CPack_Packages dist-debug dist
