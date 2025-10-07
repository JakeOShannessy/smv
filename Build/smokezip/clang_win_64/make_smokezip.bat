@echo off
cmake ^
    -B cbuild ^
    -S ../../.. ^
    -DCMAKE_C_COMPILER=clang ^
    -DCMAKE_CXX_COMPILER=clang++ ^
    -GNinja ^
    -DSTRICT_CHECKS=OFF ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DVENDORED_UI_LIBS=ON ^
    -DVENDORED_LIBS=ON
cmake --build cbuild --config Release -j6 -v --target smokezip
cmake --install cbuild --config Release --prefix dist-debug --component smokezip
