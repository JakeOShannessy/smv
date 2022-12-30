set windows-powershell := true
alias b := build
alias br := build-release

_default:
    @just --list

# Run the tests
test:
    ctest --test-dir cbuild -C Debug

# Build the debug binaries
build:
    cmake -B cbuild -DLUA=ON -DGLUI=ON -DCMAKE_BUILD_TYPE=Debug
    cmake --build cbuild --config Debug -j6 -v
    cmake --install cbuild --config Debug --prefix dist-debug

# Build the release binaries
build-release:
    cmake -B cbuild -DLUA=ON -DLUA_BUILD_BINARY=ON -DGLUI=off -DCMAKE_BUILD_TYPE=RelWithDebInfo
    cmake --build cbuild --config Release -j6 -v
    cmake --install cbuild --config Release --prefix dist

# Build release and create MSI package
package: build-release
    candle "SMVLuaInstaller.wxs"
    light "SMVLuaInstaller.wixobj"

# Clean the ./dist folder
clean-dist:
    rm -r dist

# Clean everything
clean: clean-dist
    rm -r cbuild
