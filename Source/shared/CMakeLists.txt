# smv

set(SRC_FILES
    isobox.c
    dmalloc.c
    compress.c
    file_util.c
    string_util.c
    threader.c
    histogram.c
    translate.c
    csphere.c
    md5.c
    sha1.c
    sha256.c
    stdio_m.c
    stdio_buffer.c
    getdata.c
    color2rgb.c
    colorbars.c
    colorbar_defs.c
    readimage.c
    readcad.c
    readgeom.c
    colorbars.c
    colorbar_defs.c
    readhvac.c
    readslice.c
    readsmoke.c
    readobject.c
    readcad.c
    readlabel.c
    readtour.c
    scontour2d.c
    readsmvfile.c
    "paths.c"
)

# Building libsmv as a shared library is not currently supported on Windows
if (BUILD_SHARED_LIBS AND (NOT WIN32))
    add_library(smv SHARED ${SRC_FILES})
else()
    add_library(smv STATIC ${SRC_FILES})
endif()
target_include_directories(smv PUBLIC
    .
    #TODO: this is a bit of a hack, it shouldn't really use options.h
    ../smokeview
)
if(WIN32)
    target_link_libraries(smv PUBLIC PThreads4W::PThreads4W)
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        install(DIRECTORY ${PThreads4W_DEBUG_DLL_DIR}/
            DESTINATION ${CMAKE_INSTALL_BINDIR}
            FILES_MATCHING PATTERN "*.dll")
    else()
        install(DIRECTORY ${PThreads4W_DLL_DIR}/
                DESTINATION ${CMAKE_INSTALL_BINDIR}
                FILES_MATCHING PATTERN "*.dll")
    endif()
endif()
target_link_libraries(smv PUBLIC JPEG::JPEG)
target_link_libraries(smv PUBLIC PNG::PNG)
target_link_libraries(smv PUBLIC ZLIB::ZLIB)
target_link_libraries(smv PUBLIC OpenGL::GL OpenGL::GLU)
target_link_libraries(smv PUBLIC PkgConfig::LIBGD)
if ((NOT MACOSX) AND UNIX)
    target_link_libraries(smv PUBLIC m)
endif()
if (SMV_ROOT_OVERRIDE)
    target_compile_definitions(smv PRIVATE SMV_ROOT_OVERRIDE="${SMV_ROOT_OVERRIDE}")
endif()
target_link_libraries(smv PRIVATE OpenGL::GL OpenGL::GLU)
target_compile_definitions(smv PRIVATE ASLIB)
install(TARGETS smv)
