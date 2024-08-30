
# fds2fed
add_executable(fds2fed
    main.c
    fds2fed.c
    ../shared/getdata.c
    ../shared/dmalloc.c
    ../shared/file_util.c
    ../shared/string_util.c
    ../shared/md5.c
    ../shared/sha1.c
    ../shared/sha256.c
    ../shared/stdio_buffer.c
    ../shared/isobox.c
)

target_include_directories(fds2fed PRIVATE
    .
    ../shared
)

if (LINUX)
    target_link_libraries(fds2fed PRIVATE m)
endif()
if(WIN32)
    target_link_libraries(fds2fed PRIVATE PThreads4W::PThreads4W)
endif()
