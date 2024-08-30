
# sh2bat
add_executable(sh2bat
    sh2bat.c
    ../shared/dmalloc.c
    ../shared/file_util.c
    ../shared/string_util.c
    ../shared/md5.c
    ../shared/sha1.c
    ../shared/sha256.c
    ../shared/stdio_buffer.c
)

target_include_directories(sh2bat PRIVATE
    .
    ../shared
)

if (LINUX)
    target_link_libraries(sh2bat PRIVATE m)
endif()
if(WIN32)
    target_link_libraries(sh2bat PRIVATE PThreads4W::PThreads4W)
endif()
