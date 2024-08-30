
# background
add_executable(background
    main.c
    ../shared/dmalloc.c
    ../shared/file_util.c
    ../shared/string_util.c
    ../shared/md5.c
    ../shared/sha1.c
    ../shared/sha256.c
    ../shared/stdio_buffer.c
)

target_include_directories(background PRIVATE
    .
    ../shared
)

if (LINUX)
    target_link_libraries(background PRIVATE m)
endif()
if(WIN32)
    target_link_libraries(background PRIVATE PThreads4W::PThreads4W)
endif()
