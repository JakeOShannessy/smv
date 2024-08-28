
# smvq
add_executable(smvq smvq.c
    ../shared/dmalloc.c
    ../shared/csphere.c
    ../shared/isobox.c
    ../shared/compress.c
    ../shared/file_util.c
    ../shared/string_util.c
    ../shared/threader.c
    ../shared/histogram.c
    ../shared/translate.c
    ../shared/md5.c
    ../shared/sha1.c
    ../shared/sha256.c
    ../shared/stdio_m.c
    ../shared/stdio_buffer.c
    ../shared/getdata.c
    ../shared/color2rgb.c
    ../shared/readimage.c
    ../shared/readcad.c
    ../shared/readgeom.c
    ../shared/colorbars.c
    ../shared/colorbar_defs.c
    ../shared/readhvac.c
    ../shared/readslice.c
    ../shared/readsmoke.c
    ../shared/readobject.c
    ../shared/readsmvfile.c
    ../shared/readobject.c
    ../shared/readcad.c
    ../shared/readpart.c
    ../shared/readlabel.c
    ../shared/readtour.c
)

target_include_directories(smvq PRIVATE
    ../shared
    ../glew
    ../smokeview
)

if(WIN32)
  target_link_libraries(smvq PRIVATE wsock32 ws2_32)
endif()

find_package(json-c CONFIG)
if (NOT json-c_FOUND)
    if (PKG_CONFIG_FOUND)
        pkg_check_modules(json-c json-c)
    endif()
    if (NOT json-c_FOUND)
        FetchContent_Declare(
            json-c-git
            EXCLUDE_FROM_ALL
            SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/json-c/json-c
            GIT_REPOSITORY https://github.com/json-c/json-c.git
            GIT_TAG        33337523f5cff82e659ecbe94b2fac19c7761a42
            FIND_PACKAGE_ARGS CONFIG NAMES json-c
        )
        FetchContent_MakeAvailable(json-c-git)
        target_link_libraries(smvq PRIVATE json-c-static)
        target_include_directories(smvq PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/json-c)
    else()
        message("${json-c_INCLUDE_DIRS}")
        target_link_libraries(smvq  PRIVATE ${json-c_LIBRARIES})
        target_include_directories(smvq  PRIVATE ${json-c_INCLUDE_DIRS})
        target_compile_options(smvq PUBLIC ${json-c_CFLAGS_OTHER})
    endif()
else()
    target_link_libraries(smvq PRIVATE json-c::json-c)
endif()

if(WIN32)
    target_link_libraries(smvq PRIVATE PThreads4W::PThreads4W)
endif()
# Selecting which GLUT version to use is the most platform-dependent part of the
# build.
if (MACOSX)
    add_definitions(-Dpp_NOQUARTZ)
    target_link_libraries(smvq PRIVATE "-framework OpenGL")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
endif ()
if (GLEW_FOUND)
    target_link_libraries(smvq PRIVATE GLEW::GLEW)
    # This line is a hack to work around the fact the code includes "glew.h"
    # rather than <GL/glew.h>
    target_include_directories(smvq PRIVATE ${GLEW_INCLUDE_DIRS}/GL)
else()
    target_sources(smvq PRIVATE
        ../glew/glew.c
    )
    target_include_directories(smvq PRIVATE ../glew)
endif()
if (JPEG_FOUND)
    target_link_libraries(smvq PRIVATE JPEG::JPEG)
else()
    target_link_libraries(smvq PRIVATE jpeg_static)
endif()
if (PNG_FOUND)
    target_link_libraries(smvq PRIVATE PNG::PNG)
else()
    target_link_libraries(smvq PRIVATE png_static)
endif()
if (ZLIB_FOUND)
    target_link_libraries(smvq PRIVATE ZLIB::ZLIB)
else()
    target_link_libraries(smvq PRIVATE zlib_static)
endif()
if (LIBGD_FOUND)
    target_link_libraries(smvq PRIVATE PkgConfig::LIBGD)
else()
    target_link_libraries(smvq PRIVATE gd_static)
endif()
target_link_libraries(smvq PRIVATE OpenGL::GL OpenGL::GLU)


if (WIN32)
    target_include_directories(smvq PRIVATE ../pthreads)
    find_package(unofficial-getopt-win32 REQUIRED)
    target_link_libraries(smvq PRIVATE unofficial::getopt-win32::getopt)
    install(IMPORTED_RUNTIME_ARTIFACTS unofficial::getopt-win32::getopt)
endif()
if ((NOT MACOSX) AND UNIX)
    target_link_libraries(smvq PRIVATE m)
endif()
if (LINUX)
    add_definitions(-Dpp_LINUX)
    target_link_libraries(smvq PRIVATE pthread X11 Xmu GLU GL m stdc++)
endif()
# if (LUA)
# endif()
target_link_libraries(smvq PRIVATE jsonrpc)
install(TARGETS smvq)
