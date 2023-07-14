add_library(smv STATIC
    io/common.c
    io/slcf.c
    io/bounds.c
)
target_include_directories(smv PRIVATE
    io
    ../shared
)
target_include_directories(smv PUBLIC
    .
)

if (ZLIB_FOUND)
    target_link_libraries(smv PRIVATE ZLIB::ZLIB)
else()
    target_link_libraries(smv PRIVATE zlib_static)
endif()
