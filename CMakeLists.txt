cmake_minimum_required(VERSION 3.10)
project(Smokeview LANGUAGES C CXX)

#set(CMAKE_Fortran_COMPILER C:/msys64/mingw64/bin/gfortran.exe)
#enable_language( Fortran )
add_definitions(-DBGDWIN32)

option(BUILD_SHARED_LIBS off)
option(BUILD_STATIC_LIBS on)
set(BUILD_STATIC_LIBS on force)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib" CACHE PATH "Single Directory for all Libraries")


set(CMAKE_SYSTEM_NAME Generic)
find_package(OpenGL REQUIRED)
find_package(GLUT)
if (NOT GLUT_FOUND)
    message(STATUS "Local GLUT not found, using vendored freeglut")
    set(GLUT_INCLUDE_DIR "Source/freeglut-3.2.1/include")
    set(GLUT_LIBRARIES "../Source/freeglut-3.2.1/lib/freeglut_staticd")
    set(GLUT_glut_LIBRARY_DEBUG "../Source/freeglut-3.2.1/lib/freeglut_staticd")
    set(GLUT_glut_LIBRARY_RELEASE "../Source/freeglut-3.2.1/lib/freeglut_static")
    # message( "GLUT_LIBRARIES=${GLUT_LIBRARIES}")
endif ()
include_directories( ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIR} )
add_subdirectory(Source/glui)
add_subdirectory(Source/libgd)
add_subdirectory(Source/glew-2.1.0/build/cmake)
add_subdirectory(Source/zlib-1.2.11)


option(FREEGLUT_STATIC "Statically link freeglut" on)
if (FREEGLUT_STATIC)
  add_definitions(-DFREEGLUT_STATIC)
endif ()

add_executable(Smokeview Source/smokeview/main.c
    Source/smokeview/menus.c Source/smokeview/IOscript.c Source/smokeview/IOshooter.c Source/smokeview/glui_objects.cpp Source/smokeview/glui_shooter.cpp Source/smokeview/glui_wui.cpp
    Source/shared/csphere.c Source/smokeview/colortimebar.c Source/shared/smv_endian.c Source/smokeview/camera.c Source/smokeview/IOgeometry.c Source/smokeview/IOwui.c
    Source/smokeview/IOobjects.c Source/smokeview/IOtour.c Source/smokeview/glui_display.cpp Source/smokeview/glui_tour.cpp Source/smokeview/getdatacolors.c Source/smokeview/smokeview.c
    Source/smokeview/output.c Source/smokeview/renderimage.c Source/smokeview/renderhtml.c Source/shared/isobox.c Source/smokeview/getdatabounds.c Source/smokeview/readsmv.c Source/smokeview/scontour2d.c
    Source/smokeview/glui_smoke.cpp Source/smokeview/glui_clip.cpp Source/smokeview/glui_stereo.cpp Source/smokeview/glui_geometry.cpp Source/smokeview/glui_motion.cpp
    Source/smokeview/glui_bounds.cpp Source/shared/dmalloc.c Source/shared/interp.c Source/shared/assert.c
    Source/shared/compress.c Source/smokeview/IOvolsmoke.c Source/smokeview/IOsmoke.c Source/smokeview/IOplot3d.c Source/smokeview/IOslice.c Source/smokeview/IOboundary.c
    Source/smokeview/IOpart.c Source/smokeview/IOzone.c Source/smokeview/IOiso.c Source/smokeview/callbacks.c Source/smokeview/drawGeometry.c
    Source/smokeview/glui_colorbar.cpp Source/smokeview/skybox.c Source/shared/file_util.c Source/shared/string_util.c Source/smokeview/startup.c Source/smokeview/glui_trainer.cpp
    Source/smokeview/shaders.c Source/smokeview/unit.c Source/smokeview/threader.c Source/shared/histogram.c Source/shared/translate.c Source/smokeview/update.c Source/smokeview/viewports.c
    Source/smokeview/smv_geometry.c Source/smokeview/showscene.c Source/smokeview/infoheader.c  Source/shared/md5.c Source/shared/sha1.c Source/shared/sha256.c
    Source/smokeview/fdsmodules.f90 Source/smokeview/gsmv.f90 Source/smokeview/getdata.f90
)
add_dependencies(${PROJECT_NAME} zlibstatic)
target_link_libraries(Smokeview
    libgd_static
    # freeglut-3.2.1/lib/freeglut_staticd.lib
    ${GLUT_LIBRARIES}
    # ${GLUT_glut_LIBRARY_DEBUG}
    glui_static
    glew_s
    zlibstatic
)
target_include_directories(Smokeview PRIVATE
    Source/smokeview
    Source/shared
    Source/zlib-1.2.11
    ${CMAKE_CURRENT_BINARY_DIR}/Source/zlib-1.2.11
    freeglut-3.2.1/include
    Source/libgd/src
    Source/glew-2.1.0/include/GL
    Source/glui/include/GL
)
