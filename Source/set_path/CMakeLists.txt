
if (WIN32)
    # set_path
    add_executable(set_path
        main.c
        ../shared/dmalloc.c
        ../shared/file_util.c
        ../shared/string_util.c
        ../shared/md5.c
        ../shared/sha1.c
        ../shared/sha256.c
        ../shared/stdio_buffer.c
    )

    target_include_directories(set_path PRIVATE
        .
        ../shared
    )
    target_link_libraries(set_path PRIVATE PThreads4W::PThreads4W)
endif ()
