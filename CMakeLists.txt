cmake_minimum_required(VERSION 3.21)
cmake_policy(SET CMP0091 NEW)
include(FetchContent)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
project(smv LANGUAGES C CXX)

set_property(GLOBAL PROPERTY C_STANDARD 99)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

find_package(OpenGL REQUIRED)
# message( "OPENGL_LIBRARIES=${OPENGL_LIBRARIES}")
find_package(GLUT)
find_package(GLEW)
# find_package(GLUI)
# if (NOT GLUT_FOUND)
#     #TODO need to handle debug vs release
#     message(STATUS "Local GLUT not found, using vendored freeglut")
#     set(GLUT_INCLUDE_DIR "Source/freeglut-3.2.1/include")
#     set(GLUT_LIBRARIES "Source/freeglut-3.2.1/lib/freeglut_static")
#     set(GLUT_glut_LIBRARY_DEBUG "Source/freeglut-3.2.1/lib/freeglut_staticd")
#     set(GLUT_glut_LIBRARY_RELEASE "Source/freeglut-3.2.1/lib/freeglut_static")
#     # message( "GLUT_LIBRARIES=${GLUT_LIBRARIES}")
# endif ()
# add_subdirectory(Source/freeglut-3.2.1)
# add_subdirectory(Source/gd-2.0.15)

# FetchContent_Declare(
#   GD
#   GIT_REPOSITORY https://github.com/libgd/libgd.git
#   GIT_TAG        b5319a41286107b53daa0e08e402aa1819764bdc # v2.3.3
#   OVERRIDE_FIND_PACKAGE
# )
# FetchContent_Declare(
#   Catch2
#   GIT_REPOSITORY https://github.com/catchorg/Catch2.git
#   GIT_TAG        de6fe184a9ac1a06895cdd1c9b437f0a0bdf14ad # v2.13.4
# )
# FetchContent_MakeAvailable(GD)
# find_package(GD)

# add_subdirectory(Source/zlib128)
# add_subdirectory(Source/jpeg-9b)
find_package(JPEG)
find_package(PNG)
find_package(ZLIB)
# add_subdirectory(Source/png-1.6.21)

# option(FREEGLUT_STATIC "Statically link freeglut" on)
# if (FREEGLUT_STATIC)
#     add_definitions(-DFREEGLUT_STATIC)
# endif ()

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

option(GLUI "Use glui" on)
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
    target_link_libraries(smokeview glui_static)
endif()

option(IMGUI "Use imgui" off)
if (IMGUI)
    add_definitions(-Dpp_IMGUI)
    # add_subdirectory(Libs/glui_v2_1_beta)
    set(IMGUI_DIR "../imgui")
    target_sources(smokeview PRIVATE
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_demo.cpp 
        ${IMGUI_DIR}/imgui_draw.cpp 
        ${IMGUI_DIR}/imgui_tables.cpp 
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/backends/imgui_impl_glut.cpp 
        ${IMGUI_DIR}/backends/imgui_impl_opengl2.cpp

        Source/smokeview/gui/gui.cpp
    )
    # target_link_libraries(smokeview glui_static)
    target_include_directories(smokeview PRIVATE Source/smokeview/gui ${IMGUI_DIR} ${IMGUI_DIR}/backends)
endif()

# target_include_directories(smokeview PRIVATE ${OPENGL_INCLUDE_DIRS}  )

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

# if (GD_FOUND)
#     target_link_libraries(smokeview GD_LIBRARY)
#     target_link_libraries(smokeview ${GD_LIBRARIES})
#     target_include_directories(smokeview PRIVATE ${GD_INCLUDE_DIR})
#     # message(FATAL_ERROR "No GD lib: ${GD::GD_LIBRARIES}")
#     else()
#     message(FATAL_ERROR "No GD lib")
# endif()

# target_link_libraries(smokeview GD_LIBRARY)
target_link_libraries(smokeview gd)
# message(FATAL_ERROR "No GD: ${GD_LIBRARIES}")



if (GLEW_FOUND) 
    target_link_libraries(smokeview ${GLEW_LIBRARIES})
    # target_include_directories(smokeview ${GLUT_INCLUDE_DIRS})
else()
    message(FATAL_ERROR "No GLEW lib")
endif()

if (GLUT_FOUND) 
    target_link_libraries(smokeview ${GLUT_LIBRARIES})
    # target_include_directories(smokeview ${GLUT_INCLUDE_DIRS})
else()
    message(FATAL_ERROR "No GLUT lib")
endif()

if (JPEG_FOUND) 
    target_link_libraries(smokeview ${JPEG_LIBRARIES})
else()
    message(FATAL_ERROR "No JPEG lib")
endif()

if (PNG_FOUND) 
    target_link_libraries(smokeview ${PNG_LIBRARIES})
else()
    message(FATAL_ERROR "No PNG lib")
endif()

if (ZLIB_FOUND) 
    target_link_libraries(smokeview ${ZLIB_LIBRARIES})
else()
    message(FATAL_ERROR "No ZLIB")
endif()

add_compile_definitions(pp_GITHASH="6.7.21+sc.lua")

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(MACOSX TRUE)
    add_definitions(-Dpp_OSX)
# else()
#     target_link_libraries(smokeview glut32_static)
#     add_subdirectory(Source/glut-3.7.6)
endif()


target_include_directories(smokeview PRIVATE
    Source/smokeview
    Source/shared
    Source/gd-2.0.15
    Source/glew
    Libs/glui_v2_1_beta
)

option(BETA "Include beta functionality" off)
if (BETA)
    add_compile_definitions(pp_BETA)
endif ()

option(LUA "Include lua scripting" off)
if (LUA)
    add_definitions(-Dpp_LUA)
    # add_subdirectory(Source/lua-5.3.1)
    # add_subdirectory(Source/lpeg-1.0.0)
    find_package(Lua)
    # add_subdirectory(Source/lfs/luafilesystem-1_8_0)
    # If we are including the lua interpreter, include the appropriate sources.
    target_sources(smokeview PRIVATE Source/smokeview/lua_api.c Source/smokeview/c_api.c Source/smokeview/csv.c)
    target_include_directories(smokeview PRIVATE Source/lua-5.3.1/src)
    target_link_libraries(smokeview ${LUA_LIBRARIES})

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
        # TARGETS smokeview
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
        # RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}
        # PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
    # install(
    #     TARGETS lua_shared
    #     RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}
    #     PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    # )
endif ()

install(
    TARGETS smokeview
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

if (WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_WIN32)
    add_compile_definitions(PTW32_BUILD PTW32_STATIC_LIB GLEW_STATIC X64 _WIN32 WIN32 _CONSOLE)
    target_include_directories(smokeview PRIVATE Source/pthreads)
    target_link_libraries(smokeview pthread_static)
    target_link_libraries(smokeview opengl32 glu32 gdi32 winmm comdlg32 hid)
    add_subdirectory(Source/pthreads)
    target_include_directories(smokeview PRIVATE Source/glut_gl )
else()
    target_include_directories(smokeview PRIVATE Source/glui_gl )
endif ()

if ((NOT MACOSX) AND UNIX)
    add_definitions(-Dpp_LINUX)
    target_link_libraries(smokeview pthread X11 Xmu GLU GL m stdc++)
endif()
if (MACOSX)
    add_definitions(-Dpp_NOQUARTZ)
    target_link_libraries(smokeview "-framework OpenGL" "-framework GLUT")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
endif ()

# Other programs
add_executable(smokediff Source/shared/assert.c Source/shared/dmalloc.c Source/shared/histogram.c
    Source/smokediff/IOdboundary.c Source/smokediff/IOdplot.c Source/smokediff/IOdslice.c Source/smokediff/main.c
    Source/smokediff/readsmv.c Source/shared/file_util.c Source/shared/string_util.c Source/smokediff/utilities.c
    Source/shared/md5.c Source/shared/sha1.c Source/shared/sha256.c Source/shared/stdio_buffer.c
    Source/smokeview/gsmv.c Source/smokeview/getdata.c Source/smokeview/color2rgb.c
)
target_include_directories(smokediff PRIVATE
    Source/smokediff
    Source/smokeview
    Source/shared
)

if (WIN32)
    target_include_directories(smokediff PRIVATE Source/pthreads)
endif ()

if ((NOT MACOSX) AND UNIX)
    target_link_libraries(smokediff m)
endif()


add_executable(smokezip Source/smokezip/main.c Source/smokezip/CNV3dsmoke.c Source/smokezip/CNVboundary.c
    Source/smokezip/CNVpart.c Source/smokezip/CNVplot3d.c Source/smokezip/CNVslice.c Source/shared/assert.c
    Source/shared/csphere.c Source/shared/dmalloc.c Source/smokezip/lightsmoke.c Source/shared/histogram.c
    Source/smokezip/readfiles.c Source/smokezip/utilities.c Source/shared/isobox.c Source/shared/file_util.c
    Source/shared/string_util.c Source/smokezip/threader.c Source/shared/compress.c Source/shared/md5.c
    Source/shared/sha1.c Source/shared/sha256.c Source/shared/stdio_buffer.c
    Source/smokeview/gsmv.c Source/smokeview/getdata.c
)

target_include_directories(smokezip PRIVATE
    Source/smokezip
    Source/smokeview
    Source/shared
    Source/zlib128
)

target_link_libraries(smokezip
    ${ZLIB_LIBRARIES}
)

if ((NOT MACOSX) AND UNIX)
    target_link_libraries(smokezip pthread m)
endif()

if (WIN32)
    target_include_directories(smokezip PRIVATE Source/pthreads)
    target_link_libraries(smokezip pthread_static)
endif ()

# background
add_executable(background Source/background/main.c Source/shared/assert.c Source/shared/dmalloc.c Source/shared/file_util.c
    Source/shared/string_util.c Source/shared/md5.c Source/shared/sha1.c Source/shared/sha256.c
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
add_executable(convert Source/convert/main.c Source/shared/assert.c Source/shared/dmalloc.c Source/shared/file_util.c
    Source/shared/string_util.c Source/shared/md5.c Source/shared/sha1.c Source/shared/sha256.c
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
add_executable(env2mod Source/env2mod/main.c Source/shared/assert.c Source/shared/dmalloc.c Source/shared/file_util.c
    Source/shared/string_util.c Source/shared/md5.c Source/shared/sha1.c Source/shared/sha256.c
    Source/shared/stdio_buffer.c Source/env2mod/env2mod.c
)

target_include_directories(env2mod PRIVATE
    Source/env2mod
    Source/shared
)

if ((NOT MACOSX) AND UNIX)
    target_link_libraries(env2mod m)
endif()

# flush
add_executable(flush Source/flush/main.c Source/shared/assert.c Source/shared/dmalloc.c Source/shared/file_util.c
    Source/shared/string_util.c Source/shared/md5.c Source/shared/sha1.c Source/shared/sha256.c
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
add_executable(get_time Source/get_time/get_time.c)

# getdate
add_executable(getdate Source/getdate/main.c)

target_include_directories(getdate PRIVATE Source/getdate)


# hashfile
add_executable(hashfile Source/hashfile/main.c Source/shared/assert.c Source/shared/dmalloc.c Source/shared/file_util.c
    Source/shared/string_util.c Source/shared/md5.c Source/shared/sha1.c Source/shared/sha256.c
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
add_executable(makepo Source/makepo/main.c Source/shared/assert.c Source/shared/dmalloc.c Source/shared/file_util.c
    Source/shared/string_util.c Source/shared/md5.c Source/shared/sha1.c Source/shared/sha256.c
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
add_executable(mergepo Source/mergepo/main.c Source/shared/assert.c Source/shared/dmalloc.c Source/shared/file_util.c
    Source/shared/string_util.c Source/shared/md5.c Source/shared/sha1.c Source/shared/sha256.c Source/shared/translate.c
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
add_executable(set_path Source/set_path/main.c Source/shared/assert.c Source/shared/dmalloc.c Source/shared/file_util.c
    Source/shared/string_util.c Source/shared/md5.c Source/shared/sha1.c Source/shared/sha256.c
    Source/shared/stdio_buffer.c
)

target_include_directories(set_path PRIVATE
    Source/set_path
    Source/shared
)
endif ()

# sh2bat
add_executable(sh2bat Source/sh2bat/sh2bat.c Source/shared/assert.c Source/shared/dmalloc.c Source/shared/file_util.c
    Source/shared/string_util.c Source/shared/md5.c Source/shared/sha1.c Source/shared/sha256.c
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
add_executable(timep Source/timep/main.c Source/shared/assert.c Source/shared/dmalloc.c Source/shared/file_util.c
    Source/shared/string_util.c Source/shared/md5.c Source/shared/sha1.c Source/shared/sha256.c
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
add_executable(wind2fds Source/wind2fds/main.c Source/shared/assert.c Source/shared/dmalloc.c Source/shared/file_util.c
    Source/shared/string_util.c Source/shared/md5.c Source/shared/sha1.c Source/shared/sha256.c
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
