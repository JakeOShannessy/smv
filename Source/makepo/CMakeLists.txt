
# makepo
add_executable(makepo
    main.c
    ../shared/dmalloc.c
    ../shared/file_util.c
    ../shared/string_util.c
    ../shared/md5.c
    ../shared/sha1.c
    ../shared/sha256.c
    ../shared/stdio_buffer.c
)

target_include_directories(makepo PRIVATE
    .
    ../shared
)

if (LINUX)
    target_link_libraries(makepo PRIVATE m)
endif()
if(WIN32)
    target_link_libraries(makepo PRIVATE PThreads4W::PThreads4W)
endif()
