
# env2mod
add_executable(env2mod
    main.c
    ../shared/dmalloc.c
    ../shared/file_util.c
    ../shared/string_util.c
    ../shared/md5.c
    ../shared/sha1.c
    ../shared/sha256.c
    ../shared/stdio_buffer.c
    ../env2mod/env2mod.c
)

target_include_directories(env2mod PRIVATE
    .
    ../shared
)

if (LINUX)
    target_link_libraries(env2mod PRIVATE m)
endif()
if(WIN32)
    target_link_libraries(env2mod PRIVATE PThreads4W::PThreads4W)
endif()
