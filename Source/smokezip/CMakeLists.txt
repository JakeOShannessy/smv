
# smokezip
add_executable(smokezip
    main.c
    CNV3dsmoke.c
    CNVboundary.c
    CNVpart.c
    CNVslice.c
    readfiles.c
    utilities.c
    threaderzip.c
    ../shared/dmalloc.c
    ../shared/isobox.c
    ../shared/file_util.c
    ../shared/string_util.c
    ../shared/threader.c
    ../shared/compress.c
    ../shared/md5.c
    ../shared/sha1.c
    ../shared/sha256.c
    ../shared/stdio_buffer.c
    ../shared/getdata.c
)

target_include_directories(smokezip PRIVATE
    .
    ../smokeview
    ../shared
    ../zlib128
)

if (ZLIB_FOUND)
    target_link_libraries(smokezip PRIVATE ZLIB::ZLIB)
else()
    target_link_libraries(smokezip PRIVATE zlib_static)
endif()

if (LINUX)
    target_link_libraries(smokezip PRIVATE pthread m)
endif()

if(WIN32)
    target_link_libraries(smokezip PRIVATE PThreads4W::PThreads4W)
endif()

install(TARGETS smokezip)
