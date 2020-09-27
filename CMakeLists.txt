cmake_minimum_required(VERSION 3.16)
project(Smokeview LANGUAGES C CXX Fortran)

add_definitions(-Dpp_LINUX)
remove_definitions(-Dpp_INTEL)
add_definitions(-Dpp_GCC)
set(CMAKE_Fortran_FLAGS "-cpp")

find_package(OpenGL REQUIRED)
# find_package(GLEW REQUIRED)
# find_package(FreeGLUT REQUIRED)
# find_package(GD REQUIRED)
include_directories( ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIR} )
add_subdirectory(Source/glui_v2_1_beta)
add_subdirectory(Source/gd-2.0.15)
add_subdirectory(Source/jpeg-9b)
add_subdirectory(Source/png-1.6.21)
add_subdirectory(Source/glut-3.7.6)
add_subdirectory(Source/glfw)
# add_subdirectory(Source/libui)
# add_custom_command(OUTPUT libui.a
#     COMMAND ninja -C build
#     WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Source/libui
# )

add_library(libui STATIC IMPORTED )
set_target_properties(libui PROPERTIES
        IMPORTED_LOCATION "libui.a"
        # IMPORTED_SONAME_NOCONFIG "libfoobar.so"
)
add_executable(Smokeview Source/smokeview/main.c
    Source/smokeview/menus.c Source/smokeview/IOscript.c Source/smokeview/IOshooter.c Source/smokeview/glui_objects.cpp Source/smokeview/glui_shooter.cpp Source/smokeview/glui_wui.cpp
    Source/shared/csphere.c Source/smokeview/colortimebar.c Source/shared/smv_endian.c Source/smokeview/camera.c Source/smokeview/IOgeometry.c Source/smokeview/IOwui.c
    Source/smokeview/IOobjects.c Source/smokeview/IOtour.c Source/smokeview/glui_display.cpp Source/smokeview/glui_tour.cpp Source/smokeview/getdatacolors.c Source/smokeview/smokeview.c
    Source/smokeview/output.c Source/smokeview/renderimage.c Source/smokeview/renderhtml.c Source/shared/isobox.c Source/smokeview/getdatabounds.c Source/smokeview/readsmv.c Source/smokeview/scontour2d.c
    Source/smokeview/glui_smoke.cpp Source/smokeview/glui_clip.cpp Source/smokeview/glui_stereo.cpp Source/smokeview/glui_geometry.cpp Source/smokeview/glui_motion.cpp
    Source/smokeview/glui_bounds.cpp Source/shared/dmalloc.c Source/shared/assert.c Source/shared/stdio_m.c
    Source/shared/compress.c Source/smokeview/IOvolsmoke.c Source/smokeview/IOsmoke.c Source/smokeview/IOplot3d.c Source/smokeview/IOslice.c Source/smokeview/IOboundary.c
    Source/smokeview/IOpart.c Source/smokeview/IOzone.c Source/smokeview/IOiso.c Source/smokeview/callbacks.c Source/smokeview/drawGeometry.c
    Source/smokeview/glui_colorbar.cpp Source/smokeview/skybox.c Source/shared/file_util.c Source/shared/string_util.c Source/smokeview/startup.c Source/smokeview/glui_trainer.cpp
    Source/smokeview/shaders.c Source/smokeview/unit.c Source/smokeview/threader.c Source/shared/histogram.c Source/shared/translate.c Source/smokeview/update.c Source/smokeview/viewports.c
    Source/smokeview/smv_geometry.c Source/smokeview/showscene.c Source/smokeview/infoheader.c  Source/shared/md5.c Source/shared/sha1.c Source/shared/sha256.c
    Source/smokeview/fdsmodules.f90 Source/smokeview/gsmv.f90 Source/smokeview/getdata.f90
    Source/glew/glew.c
)

option(LUA "Include lua scripting" off)
if (LUA)
    add_definitions(-Dpp_LUA)
    # If we are including the lua interpreter, include the appropriate sources.
    target_sources(Smokeview PRIVATE Source/smokeview/lua_api.c Source/smokeview/c_api.c Source/smokeview/csv.c)
    target_link_libraries(Smokeview liblua)
endif ()

option(MULTITHREADED "Use multithreading via pthreads" on)
if (MULTITHREADED)
    # add_definitions(-Dpp_THREAD)
    #TODO: throw an appropriate error message if MULTITHREADED is enabled but we can't use pthreads.
endif ()

target_link_libraries(Smokeview
    glui_static
    opengl32

    winglut
    glu32
    gd

    jpeg
    png

    glfw
    libui

    gdi32
    winmm
    comdlg32
    comctl32
    ole32
    d2d1
    dwrite

    gfortran
    "-Wl,-Bstatic -lquadmath"
    "-Wl,-Bstatic -lpthread"
    "-Wl,-Bstatic -lz"
)
# add_dependencies(Smokeview libui.a)
add_definitions(-Dpp_LINUX)
add_definitions(-DGLEW_STATIC)
add_definitions(-DMINGW)
add_definitions(-D__MINGW32__)
add_definitions(-DWIN32)
add_definitions(-D_WIN32)
target_compile_options(Smokeview PRIVATE -Wno-write-strings)
target_link_options(Smokeview PRIVATE -static-libgcc -static-libstdc++)
target_include_directories(Smokeview PRIVATE
    ${GLEW_INCLUDE_DIRS}/GL
    Source/smokeview
    Source/shared
    Source/glew
    Source/jpeg-9b

    Source/glui_v2_1_beta
    Source/lua-5.3.1/src
    /mingw64/include/GL/
    Source/gd-2.0.15
    Source/glut-3.7.6
    Source/glui_gl
    Source/glfw/include
    Source/libui
)
