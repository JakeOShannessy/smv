cmake_minimum_required(VERSION 3.23)
cmake_policy(SET CMP0091 NEW)
include(FetchContent)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
project(smv LANGUAGES C CXX)
set_property(GLOBAL PROPERTY C_STANDARD 99)

# Options which can be configured (along with their defaults)
option(GLUI "Use glui" on)
option(BETA "Include beta functionality" off)
option(LUA "Include lua scripting" off)
option(LUA_BUILD_BINARY "Build a lua intepreter" ON)
option(STRICT_CHECKS "Run additional strict checks" OFF)
option(IMGUI "Use imgui" off)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(MACOSX TRUE)
    add_definitions(-Dpp_OSX)
endif()

if(STRICT_CHECKS)
    set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-checks=clang-diagnostic-*,clang-analyzer-*,-clang-analyzer-security.insecureAPI.*")
    set(CMAKE_C_CLANG_TIDY "clang-tidy;-checks=clang-diagnostic-*,clang-analyzer-*,-clang-analyzer-security.insecureAPI.*")
    add_compile_options(-O2)
    add_compile_options(-flto=auto)
    add_compile_options(-ffat-lto-objects)
    add_compile_options(-fexceptions)
    add_compile_options(-grecord-gcc-switches)
    add_compile_options(-Wall)
    add_compile_options(-Werror=format-security)
    add_compile_options(-Wp,-D_FORTIFY_SOURCE=2)
    add_compile_options(-Wp,-D_GLIBCXX_ASSERTIONS )
    add_compile_options(-fstack-protector-strong)
    add_compile_options(-fasynchronous-unwind-tables)
    add_compile_options(-fstack-clash-protection -fcf-protection)
    add_compile_options(-fcf-protection)
endif()

find_package(OpenGL REQUIRED)
if (WIN32)
    find_package(FreeGLUT CONFIG REQUIRED)
else()
    find_package(GLUT REQUIRED)
endif()
find_package(GLEW REQUIRED)
find_package(JPEG REQUIRED)
find_package(PNG REQUIRED)
find_package(ZLIB REQUIRED)

add_executable(smokeview
    Source/smokeview/main.c
    Source/smokeview/menus.c
    Source/smokeview/IOscript.c
    Source/smokeview/IOshooter.c
    Source/shared/csphere.c
    Source/smokeview/colortimebar.c
    Source/smokeview/camera.c
    Source/smokeview/IOgeometry.c
    Source/smokeview/IOwui.c
    Source/smokeview/IOobjects.c
    Source/smokeview/IOtour.c
    Source/smokeview/getdatacolors.c
    Source/smokeview/smokeview.c
    Source/smokeview/output.c
    Source/smokeview/renderimage.c
    Source/smokeview/renderhtml.c
    Source/shared/isobox.c
    Source/smokeview/getdatabounds.c
    Source/smokeview/readsmv.c
    Source/smokeview/scontour2d.c
    Source/shared/dmalloc.c
    Source/shared/assert.c
    Source/shared/compress.c
    Source/smokeview/IOvolsmoke.c
    Source/smokeview/IOsmoke.c
    Source/smokeview/IOplot3d.c
    Source/smokeview/IOplot2d.c
    Source/smokeview/IOslice.c
    Source/smokeview/IOhvac.c
    Source/smokeview/IOboundary.c
    Source/smokeview/IOpart.c
    Source/smokeview/IOzone.c
    Source/smokeview/IOiso.c
    Source/smokeview/callbacks.c
    Source/smokeview/drawGeometry.c
    Source/smokeview/skybox.c
    Source/shared/file_util.c
    Source/shared/string_util.c
    Source/smokeview/startup.c
    Source/smokeview/shaders.c
    Source/smokeview/unit.c
    Source/smokeview/threader.c
    Source/shared/histogram.c
    Source/shared/translate.c
    Source/smokeview/update.c
    Source/smokeview/viewports.c
    Source/smokeview/smv_geometry.c
    Source/smokeview/showscene.c
    Source/smokeview/infoheader.c
    Source/shared/md5.c
    Source/shared/sha1.c
    Source/shared/sha256.c
    Source/shared/stdio_m.c
    Source/shared/stdio_buffer.c
    Source/smokeview/gsmv.c
    Source/smokeview/getdata.c
    Source/smokeview/color2rgb.c
    Source/smokeview/colortable.c
)

if (GLUI)
    add_definitions(-Dpp_GLUI)
    add_subdirectory(Libs/glui_v2_1_beta)
    target_sources(smokeview PRIVATE
        Source/smokeview/glui_smoke.cpp
        Source/smokeview/glui_clip.cpp
        Source/smokeview/glui_stereo.cpp
        Source/smokeview/glui_geometry.cpp
        Source/smokeview/glui_motion.cpp
        Source/smokeview/glui_bounds.cpp
        Source/smokeview/glui_colorbar.cpp
        Source/smokeview/glui_display.cpp
        Source/smokeview/glui_tour.cpp
        Source/smokeview/glui_trainer.cpp
        Source/smokeview/glui_objects.cpp
        Source/smokeview/glui_shooter.cpp
    )
    target_link_libraries(smokeview PRIVATE glui_static)
endif()

if (IMGUI)
    # FetchContent_Declare(imgui
    #     GIT_REPOSITORY https://github.com/ocornut/imgui.git
    #     GIT_TAG        a8df192df022ed6ac447e7b7ada718c4c4824b41 # v1.89.1
    # )
    # FetchContent_GetProperties(imgui)
    #     if(NOT imgui_POPULATED)
    #         message("Cloning imgui")
    #         FetchContent_Populate(imgui)
    # endif()
    set(imgui_SOURCE_DIR "../imgui")
    add_definitions(-Dpp_IMGUI)
    target_sources(smokeview PRIVATE
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp 
        ${imgui_SOURCE_DIR}/imgui_draw.cpp 
        ${imgui_SOURCE_DIR}/imgui_tables.cpp 
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glut.cpp 
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl2.cpp
        Source/smokeview/gui/gui.cpp
    )
    target_include_directories(smokeview PRIVATE Source/smokeview/gui ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
endif()

set(SMV_LUA_CORE
    Source/smvluacore/smv.lua Source/smvluacore/ssf.lua
    Source/smvluacore/ssfparser.lua Source/smvluacore/ssfcommands.lua
    Source/smvluacore/clipping.lua Source/smvluacore/bounds.lua
    Source/smvluacore/render.lua Source/smvluacore/load.lua
    Source/smvluacore/view.lua Source/smvluacore/tour.lua
    Source/smvluacore/iniparser.lua Source/smvluacore/inioptions.lua
    Source/smvluacore/unload.lua Source/smvluacore/constants.lua
    Source/smvluacore/camera.lua Source/smvluacore/window.lua
)

add_definitions(-Dpp_CMAKE)

target_link_libraries(smokeview PRIVATE GLEW::GLEW)
# GLUT/FreeGLUT
if (WIN32)
    target_link_libraries(smokeview PRIVATE $<IF:$<TARGET_EXISTS:FreeGLUT::freeglut>,FreeGLUT::freeglut,FreeGLUT::freeglut_static>)
else()
    target_link_libraries(smokeview PRIVATE GLUT::GLUT)
endif()
target_link_libraries(smokeview PRIVATE JPEG::JPEG)
target_link_libraries(smokeview PRIVATE PNG::PNG)
target_link_libraries(smokeview PRIVATE ZLIB::ZLIB)
find_package(PkgConfig)
pkg_check_modules(LIBGD REQUIRED IMPORTED_TARGET gdlib)
target_link_libraries(smokeview PRIVATE PkgConfig::LIBGD)
if (WIN32) 
    find_package(PThreads4W REQUIRED)
    target_link_libraries(smokeview PRIVATE PThreads4W::PThreads4W)
endif()

add_compile_definitions(pp_GITHASH="6.7.21+sc.lua")

target_include_directories(smokeview PRIVATE
    Source/smokeview
    Source/shared
    Libs/glui_v2_1_beta
)

if (BETA)
    add_compile_definitions(pp_BETA)
endif ()

if (LUA)
    add_definitions(-Dpp_LUA)
    find_package(Lua)
    target_link_libraries(smokeview PRIVATE ${LUA_LIBRARIES})
    target_include_directories(smokeview PRIVATE ${LUA_INCLUDE_DIR})
    target_sources(smokeview PRIVATE 
        Source/smokeview/lua_api.c 
        Source/smokeview/c_api.c 
        Source/smokeview/csv.c
    )

    target_sources(smokeview PUBLIC
        Source/smvluacore/smv.lua Source/smvluacore/ssf.lua
        Source/smvluacore/ssfparser.lua Source/smvluacore/ssfcommands.lua
        Source/smvluacore/clipping.lua Source/smvluacore/bounds.lua
        Source/smvluacore/render.lua Source/smvluacore/load.lua
        Source/smvluacore/view.lua Source/smvluacore/tour.lua
        Source/smvluacore/iniparser.lua Source/smvluacore/inioptions.lua
        Source/smvluacore/unload.lua Source/smvluacore/constants.lua
        Source/smvluacore/gnuplot.lua Source/smvluacore/plot.lua
        Source/smvluacore/pl3d.lua Source/smvluacore/json.lua
        Source/smvluacore/camera.lua Source/smvluacore/window.lua
    )
    install(
        FILES
        Source/smvluacore/smv.lua Source/smvluacore/ssf.lua
        Source/smvluacore/ssfparser.lua Source/smvluacore/ssfcommands.lua
        Source/smvluacore/clipping.lua Source/smvluacore/bounds.lua
        Source/smvluacore/render.lua Source/smvluacore/load.lua
        Source/smvluacore/view.lua Source/smvluacore/tour.lua
        Source/smvluacore/iniparser.lua Source/smvluacore/inioptions.lua
        Source/smvluacore/unload.lua Source/smvluacore/constants.lua
        Source/smvluacore/gnuplot.lua Source/smvluacore/plot.lua
        Source/smvluacore/pl3d.lua Source/smvluacore/json.lua
        Source/smvluacore/camera.lua Source/smvluacore/window.lua
        DESTINATION bin
    )

    if (WIN32)
        FetchContent_Declare(lfs
            GIT_REPOSITORY https://github.com/lunarmodules/luafilesystem.git
            GIT_TAG        7c6e1b013caec0602ca4796df3b1d7253a2dd258 # v1.8.0
        )
        FetchContent_GetProperties(lfs)
            if(NOT lfs_POPULATED)
                message("Cloning LFS")
                FetchContent_Populate(lfs)
        endif()

        add_library(lfs_shared SHARED
            ${lfs_SOURCE_DIR}/src/lfs.c
        )
        target_link_libraries(lfs_shared PRIVATE ${LUA_LIBRARIES})
        target_include_directories(lfs_shared PRIVATE ${LUA_INCLUDE_DIR})
        set_target_properties(lfs_shared PROPERTIES OUTPUT_NAME lfs)
        install(TARGETS lfs_shared)

        FetchContent_Declare(lpeg
            URL http://www.inf.puc-rio.br/~roberto/lpeg/lpeg-1.0.2.tar.gz
            URL_HASH     MD5=d342571886f1abcb7afe6a83d024d583 # v1.0.2
        )
        FetchContent_GetProperties(lpeg)
            if(NOT lpeg_POPULATED)
                message("Cloning LPEG")
                FetchContent_Populate(lpeg)
        endif()

        add_library(lpeg_shared SHARED
            ${lpeg_SOURCE_DIR}/lpvm.c
            ${lpeg_SOURCE_DIR}/lpcap.c
            ${lpeg_SOURCE_DIR}/lptree.c
            ${lpeg_SOURCE_DIR}/lpcode.c
            ${lpeg_SOURCE_DIR}/lpprint.c
        )
        target_link_libraries(lpeg_shared PRIVATE ${LUA_LIBRARIES})
        target_include_directories(lpeg_shared PRIVATE ${LUA_INCLUDE_DIR})
        set_target_properties(lpeg_shared PROPERTIES OUTPUT_NAME lpeg)
        install(TARGETS lpeg_shared)


        FetchContent_Declare(lua_exe
            GIT_REPOSITORY https://github.com/lua/lua.git
            GIT_TAG        5d708c3f9cae12820e415d4f89c9eacbe2ab964b # v5.4.4
        )
        FetchContent_GetProperties(lua_exe)
            if(NOT lua_exe)
                message("Cloning lua_exe")
                FetchContent_Populate(lua_exe)
        endif()
        if(LUA_BUILD_BINARY)
            add_executable(lua_exe
                ${lua_exe_SOURCE_DIR}/lapi.c
                ${lua_exe_SOURCE_DIR}/lauxlib.c
                ${lua_exe_SOURCE_DIR}/lbaselib.c
                ${lua_exe_SOURCE_DIR}/lcode.c
                ${lua_exe_SOURCE_DIR}/lcorolib.c
                ${lua_exe_SOURCE_DIR}/lctype.c
                ${lua_exe_SOURCE_DIR}/ldblib.c
                ${lua_exe_SOURCE_DIR}/ldebug.c
                ${lua_exe_SOURCE_DIR}/ldo.c
                ${lua_exe_SOURCE_DIR}/ldump.c
                ${lua_exe_SOURCE_DIR}/lfunc.c
                ${lua_exe_SOURCE_DIR}/lgc.c
                ${lua_exe_SOURCE_DIR}/linit.c
                ${lua_exe_SOURCE_DIR}/liolib.c
                ${lua_exe_SOURCE_DIR}/llex.c
                ${lua_exe_SOURCE_DIR}/lmathlib.c
                ${lua_exe_SOURCE_DIR}/lmem.c
                ${lua_exe_SOURCE_DIR}/loadlib.c
                ${lua_exe_SOURCE_DIR}/lobject.c
                ${lua_exe_SOURCE_DIR}/lopcodes.c
                ${lua_exe_SOURCE_DIR}/loslib.c
                ${lua_exe_SOURCE_DIR}/lparser.c
                ${lua_exe_SOURCE_DIR}/lstate.c
                ${lua_exe_SOURCE_DIR}/lstring.c
                ${lua_exe_SOURCE_DIR}/lstrlib.c
                ${lua_exe_SOURCE_DIR}/ltable.c
                ${lua_exe_SOURCE_DIR}/ltablib.c
                ${lua_exe_SOURCE_DIR}/ltm.c
                ${lua_exe_SOURCE_DIR}/lua.c
                ${lua_exe_SOURCE_DIR}/lundump.c
                ${lua_exe_SOURCE_DIR}/lutf8lib.c
                ${lua_exe_SOURCE_DIR}/lvm.c
                ${lua_exe_SOURCE_DIR}/lzio.c
            )
            if(NOT(WIN32))
                # Build with readline when not on windows
                pkg_check_modules(Readline REQUIRED IMPORTED_TARGET readline)
                target_link_libraries(lua_exe PRIVATE PkgConfig::Readline)
                install(TARGETS lua_exe)
            endif()
        endif()
    endif()
endif ()

install(
    TARGETS smokeview
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

if (WIN32)
    add_compile_definitions(X64 _WIN32 WIN32 _CONSOLE BGDWIN32 NONDLL)
endif ()

if ((NOT MACOSX) AND UNIX)
    add_definitions(-Dpp_LINUX)
    target_link_libraries(smokeview PRIVATE pthread X11 Xmu GLU GL m stdc++)
endif()
if (MACOSX)
    add_definitions(-Dpp_NOQUARTZ)
    target_link_libraries(smokeview PRIVATE "-framework OpenGL" "-framework GLUT")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
endif ()

# Other programs
add_executable(smokediff 
    Source/shared/assert.c 
    Source/shared/dmalloc.c 
    Source/shared/histogram.c
    Source/smokediff/IOdboundary.c 
    Source/smokediff/IOdplot.c 
    Source/smokediff/IOdslice.c 
    Source/smokediff/main.c
    Source/smokediff/readsmv.c 
    Source/shared/file_util.c 
    Source/shared/string_util.c 
    Source/smokediff/utilities.c
    Source/shared/md5.c 
    Source/shared/sha1.c 
    Source/shared/sha256.c 
    Source/shared/stdio_buffer.c
    Source/smokeview/gsmv.c 
    Source/smokeview/getdata.c 
    Source/smokeview/color2rgb.c
)
target_include_directories(smokediff PRIVATE
    Source/smokediff
    Source/smokeview
    Source/shared
)

if(WIN32)
    target_link_libraries(smokediff PRIVATE PThreads4W::PThreads4W)
endif()

if ((NOT MACOSX) AND UNIX)
    target_link_libraries(smokediff m)
endif()


add_executable(smokezip 
    Source/smokezip/main.c 
    Source/smokezip/CNV3dsmoke.c 
    Source/smokezip/CNVboundary.c
    Source/smokezip/CNVpart.c 
    Source/smokezip/CNVplot3d.c 
    Source/smokezip/CNVslice.c 
    Source/shared/assert.c
    Source/shared/csphere.c 
    Source/shared/dmalloc.c 
    Source/smokezip/lightsmoke.c 
    Source/shared/histogram.c
    Source/smokezip/readfiles.c 
    Source/smokezip/utilities.c 
    Source/shared/isobox.c 
    Source/shared/file_util.c
    Source/shared/string_util.c 
    Source/smokezip/threader.c 
    Source/shared/compress.c 
    Source/shared/md5.c
    Source/shared/sha1.c 
    Source/shared/sha256.c 
    Source/shared/stdio_buffer.c
    Source/smokeview/gsmv.c 
    Source/smokeview/getdata.c
)

target_include_directories(smokezip PRIVATE
    Source/smokezip
    Source/smokeview
    Source/shared
    Source/zlib128
)

target_link_libraries(smokezip PRIVATE ZLIB::ZLIB)
if(WIN32)
    target_link_libraries(smokezip PRIVATE PThreads4W::PThreads4W)
endif()

if ((NOT MACOSX) AND UNIX)
    target_link_libraries(smokezip PRIVATE pthread m)
endif()


# background
add_executable(background 
    Source/background/main.c 
    Source/shared/assert.c 
    Source/shared/dmalloc.c 
    Source/shared/file_util.c
    Source/shared/string_util.c 
    Source/shared/md5.c 
    Source/shared/sha1.c 
    Source/shared/sha256.c
    Source/shared/stdio_buffer.c
)

target_include_directories(background PRIVATE
    Source/background
    Source/shared
)

if ((NOT MACOSX) AND UNIX)
    target_link_libraries(background m)
endif()

# convert
add_executable(convert 
    Source/convert/main.c 
    Source/shared/assert.c 
    Source/shared/dmalloc.c 
    Source/shared/file_util.c
    Source/shared/string_util.c 
    Source/shared/md5.c 
    Source/shared/sha1.c 
    Source/shared/sha256.c
    Source/shared/stdio_buffer.c
)

target_include_directories(convert PRIVATE
    Source/convert
    Source/shared
)

if ((NOT MACOSX) AND UNIX)
    target_link_libraries(convert m)
endif()

# env2mod
add_executable(env2mod 
    Source/env2mod/main.c 
    Source/shared/assert.c 
    Source/shared/dmalloc.c 
    Source/shared/file_util.c
    Source/shared/string_util.c 
    Source/shared/md5.c 
    Source/shared/sha1.c 
    Source/shared/sha256.c
    Source/shared/stdio_buffer.c 
    Source/env2mod/env2mod.c
)

target_include_directories(env2mod PRIVATE
    Source/env2mod
    Source/shared
)

if ((NOT MACOSX) AND UNIX)
    target_link_libraries(env2mod m)
endif()

# flush
add_executable(flush 
    Source/flush/main.c 
    Source/shared/assert.c 
    Source/shared/dmalloc.c 
    Source/shared/file_util.c
    Source/shared/string_util.c 
    Source/shared/md5.c 
    Source/shared/sha1.c 
    Source/shared/sha256.c
    Source/shared/stdio_buffer.c
)

target_include_directories(flush PRIVATE
    Source/flush
    Source/shared
)

if ((NOT MACOSX) AND UNIX)
    target_link_libraries(flush m)
endif()

# get_time
add_executable(get_time 
    Source/get_time/get_time.c
)

# getdate
add_executable(getdate 
    Source/getdate/main.c
)

target_include_directories(getdate PRIVATE Source/getdate)


# hashfile
add_executable(hashfile 
    Source/hashfile/main.c 
    Source/shared/assert.c 
    Source/shared/dmalloc.c 
    Source/shared/file_util.c
    Source/shared/string_util.c 
    Source/shared/md5.c 
    Source/shared/sha1.c 
    Source/shared/sha256.c
    Source/shared/stdio_buffer.c
)

target_include_directories(hashfile PRIVATE
    Source/hashfile
    Source/shared
)

if ((NOT MACOSX) AND UNIX)
    target_link_libraries(hashfile m)
endif()

# makepo
add_executable(makepo 
    Source/makepo/main.c 
    Source/shared/assert.c 
    Source/shared/dmalloc.c 
    Source/shared/file_util.c
    Source/shared/string_util.c 
    Source/shared/md5.c 
    Source/shared/sha1.c 
    Source/shared/sha256.c
    Source/shared/stdio_buffer.c
)

target_include_directories(makepo PRIVATE
    Source/makepo
    Source/shared
)

if ((NOT MACOSX) AND UNIX)
    target_link_libraries(makepo m)
endif()

# mergepo
add_executable(mergepo 
    Source/mergepo/main.c 
    Source/shared/assert.c 
    Source/shared/dmalloc.c 
    Source/shared/file_util.c
    Source/shared/string_util.c
    Source/shared/md5.c 
    Source/shared/sha1.c 
    Source/shared/sha256.c 
    Source/shared/translate.c
    Source/shared/stdio_buffer.c
)

target_include_directories(mergepo PRIVATE
    Source/mergepo
    Source/shared
)

if ((NOT MACOSX) AND UNIX)
    target_link_libraries(mergepo m)
endif()

if (WIN32)
    # set_path
    add_executable(set_path 
        Source/set_path/main.c 
        Source/shared/assert.c 
        Source/shared/dmalloc.c
        Source/shared/file_util.c
        Source/shared/string_util.c 
        Source/shared/md5.c 
        Source/shared/sha1.c 
        Source/shared/sha256.c
        Source/shared/stdio_buffer.c
    )

    target_include_directories(set_path PRIVATE
        Source/set_path
        Source/shared
    )
endif ()

# sh2bat
add_executable(sh2bat 
    Source/sh2bat/sh2bat.c 
    Source/shared/assert.c 
    Source/shared/dmalloc.c 
    Source/shared/file_util.c
    Source/shared/string_util.c 
    Source/shared/md5.c 
    Source/shared/sha1.c 
    Source/shared/sha256.c
    Source/shared/stdio_buffer.c
)

target_include_directories(sh2bat PRIVATE
    Source/sh2bat
    Source/shared
)

if ((NOT MACOSX) AND UNIX)
    target_link_libraries(sh2bat m)
endif()

# timep
add_executable(timep 
    Source/timep/main.c 
    Source/shared/assert.c
    Source/shared/dmalloc.c 
    Source/shared/file_util.c
    Source/shared/string_util.c 
    Source/shared/md5.c 
    Source/shared/sha1.c 
    Source/shared/sha256.c
    Source/shared/stdio_buffer.c
)

target_include_directories(timep PRIVATE
    Source/timep
    Source/shared
)

if ((NOT MACOSX) AND UNIX)
    target_link_libraries(timep m)
endif()

# wind2fds
add_executable(wind2fds 
    Source/wind2fds/main.c 
    Source/shared/assert.c 
    Source/shared/dmalloc.c 
    Source/shared/file_util.c
    Source/shared/string_util.c 
    Source/shared/md5.c 
    Source/shared/sha1.c 
    Source/shared/sha256.c
    Source/shared/stdio_buffer.c
)

target_include_directories(wind2fds PRIVATE
    Source/wind2fds
    Source/shared
)

if ((NOT MACOSX) AND UNIX)
    target_link_libraries(wind2fds m)
endif()

include(CTest)
enable_testing()
add_subdirectory(Tests)
add_subdirectory(Verification/Visualization)
