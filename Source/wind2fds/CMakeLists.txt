# wind2fds
add_executable(wind2fds main.c)
target_link_libraries(wind2fds PRIVATE libsmv)

if (COVERAGE)
    target_compile_options(wind2fds PRIVATE -coverage -O0 -fprofile-arcs -ftest-coverage)
    target_link_options(wind2fds PRIVATE -lgcov --coverage)
endif()

install(TARGETS wind2fds)
