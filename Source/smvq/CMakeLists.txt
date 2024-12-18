
# smvq
add_executable(smvq smvq.c
    ../shared/readsmvfile.c
)

target_link_libraries(smvq PRIVATE libsmv)

if (WIN32)
    find_package(unofficial-getopt-win32 REQUIRED)
    target_link_libraries(smvq PRIVATE unofficial::getopt-win32::getopt)
    # if (BUILD_SHARED_LIBS)
    #     install(IMPORTED_RUNTIME_ARTIFACTS unofficial::getopt-win32::getopt)
    # endif()
endif()
target_link_libraries(smvq PUBLIC json-c::json-c)
install(TARGETS smvq)
