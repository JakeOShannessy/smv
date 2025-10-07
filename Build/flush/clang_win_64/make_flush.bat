@echo off
cmake ^
    -B cbuild ^
    -S ../../.. ^
    -DCMAKE_C_COMPILER=clang ^
    -DCMAKE_CXX_COMPILER=clang++ ^
    -GNinja ^
    -DSTRICT_CHECKS=OFF ^
    -DCMAKE_BUILD_TYPE=Debug ^
    -DVENDORED_UI_LIBS=ON ^
    -DVENDORED_LIBS=ON
cmake --build cbuild --config Debug -j6 -v --target flush
cmake --install cbuild --config Debug --prefix dist-debug --component flush
