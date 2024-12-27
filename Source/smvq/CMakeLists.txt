# smvq
if (NOT EMSCRIPTEN)
    add_executable(smvq smvq.c)

    target_include_directories(smvq PRIVATE)
    target_link_libraries(smvq PRIVATE libsmv)

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
    if (WIN32)
        find_package(unofficial-getopt-win32 REQUIRED)
        target_link_libraries(smvq PRIVATE unofficial::getopt-win32::getopt)
        # if (BUILD_SHARED_LIBS)
        #     install(IMPORTED_RUNTIME_ARTIFACTS unofficial::getopt-win32::getopt)
        # endif()
    endif()
    target_link_libraries(smvq PUBLIC json-c::json-c)
    install(TARGETS smvq)
endif()



if (EMSCRIPTEN)
    # smvq-wasm
    add_executable(smvq-wasm
        smvq.c
        # ../shared/histogram.c
        # ../shared/readtour.c
        # ../shared/readlabel.c
        # ../shared/stdio_m.c
        # ../shared/color2rgb.c
        # ../shared/dmalloc.c
        # ../shared/readcad.c
        # ../shared/scontour2d.c
        # ../shared/colorbar_defs.c
        # ../shared/IOframe.c
        # ../shared/readobject.c
        # ../shared/string_util.c
        # ../shared/colorbars.c
        # ../shared/readgeom.c
        # ../shared/readimage.c
        # ../shared/sha1.c
        # ../shared/isobox.c
        # ../shared/readslice.c
        # ../shared/sha256.c
        # ../shared/threader.c
        # ../shared/compress.c
        # ../shared/fopen.c
        # ../shared/readhvac.c
        # ../shared/readsmoke.c
        # ../shared/smokestream.c
        # ../shared/translate.c
        # ../shared/csphere.c
        # ../shared/getdata.c
        # ../shared/md5.c
        # ../shared/readsmvfile.c
        # ../shared/stdio_buffer.c
        # ../shared/file_util.c
    )

    target_link_libraries(smvq-wasm PRIVATE libsmv)
    # target_include_directories(smvq-wasm PRIVATE
    #     ../shared
    #     ../smokeview
    #     ~/json-c/build
    #     ~
    # )
    FetchContent_MakeAvailable(json-c)
    target_link_libraries(smvq-wasm PRIVATE json-c)
    # target_link_options(smvq-wasm PRIVATE --embed-file "${CMAKE_SOURCE_DIR}/couch.smv@couch.smv")

    FetchContent_MakeAvailable(libgd)
    target_link_options(smvq-wasm PRIVATE -sUSE_LIBJPEG=1)
    target_link_options(smvq-wasm PRIVATE -sUSE_LIBPNG=1)
    target_link_options(smvq-wasm PRIVATE -sUSE_ZLIB=1)
    target_link_options(smvq-wasm PRIVATE -lnodefs.js)
    target_link_options(smvq-wasm PRIVATE -sEXPORTED_FUNCTIONS='_RunSmvq','_free')
    target_link_options(smvq-wasm PRIVATE -sEXPORTED_RUNTIME_METHODS='ccall','cwrap')
    target_link_libraries(smvq-wasm PRIVATE "${libgd_BINARY_DIR}/Bin/libgd.a")
    target_include_directories(smvq-wasm PRIVATE ${libgd_SOURCE_DIR}/src)
endif()
