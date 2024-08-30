# smokediff
add_executable(smokediff
    main.c
    IOdboundary.c
    IOdplot.c
    IOdslice.c
    readsmv.c
    utilities.c
    ../shared/dmalloc.c
    ../shared/histogram.c
    ../shared/file_util.c
    ../shared/string_util.c
    ../shared/md5.c
    ../shared/sha1.c
    ../shared/sha256.c
    ../shared/stdio_buffer.c
    ../shared/getdata.c
    ../shared/color2rgb.c
)
target_include_directories(smokediff PRIVATE
    .
    ../smokeview
    ../shared
)

if(WIN32)
    target_link_libraries(smokediff PRIVATE PThreads4W::PThreads4W)
endif()

if (LINUX)
    target_link_libraries(smokediff PRIVATE m)
endif()

install(TARGETS smokediff)
