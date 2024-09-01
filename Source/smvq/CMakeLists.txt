
# smvq
add_executable(smvq smvq.c
    ../shared/readsmvfile.c
)

target_include_directories(smvq PRIVATE
    ../shared
    ../smokeview
)

target_link_libraries(smvq PRIVATE libsmv)
if(WIN32)
  target_link_libraries(smvq PRIVATE wsock32 ws2_32)
endif()

# Selecting which GLUT version to use is the most platform-dependent part of the
# build.
if (MACOSX)
    add_definitions(-Dpp_NOQUARTZ)
    target_link_libraries(smvq PRIVATE "-framework OpenGL")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
endif ()
target_link_libraries(smvq PRIVATE GLEW::GLEW)
# This line is a hack to work around the fact the code includes "glew.h"
# rather than <GL/glew.h>
target_include_directories(smvq PRIVATE ${GLEW_INCLUDE_DIRS}/GL)


if (WIN32)
    find_package(unofficial-getopt-win32 REQUIRED)
    target_link_libraries(smvq PRIVATE unofficial::getopt-win32::getopt)
    # if (BUILD_SHARED_LIBS)
    #     install(IMPORTED_RUNTIME_ARTIFACTS unofficial::getopt-win32::getopt)
    # endif()
endif()
if ((NOT MACOSX) AND UNIX)
    target_link_libraries(smvq PRIVATE m)
endif()
if (LINUX)
    add_definitions(-Dpp_LINUX)
    target_link_libraries(smvq PRIVATE pthread X11 Xmu GLU GL m stdc++)
endif()
target_link_libraries(smvq PRIVATE jsonrpc)
install(TARGETS smvq)
