
# flush
add_executable(flush
    main.c
    ../shared/dmalloc.c
    ../shared/file_util.c
    ../shared/string_util.c
    ../shared/md5.c
    ../shared/sha1.c
    ../shared/sha256.c
    ../shared/stdio_buffer.c
)

target_include_directories(flush PRIVATE
    .
    ../shared
)

if (LINUX)
    target_link_libraries(flush PRIVATE m)
endif()
if(WIN32)
    target_link_libraries(flush PRIVATE PThreads4W::PThreads4W)
endif()
