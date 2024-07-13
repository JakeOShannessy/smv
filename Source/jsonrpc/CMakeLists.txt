# We select CMake version 3.25 as it allows us to use the LINUX variable.
cmake_minimum_required(VERSION 3.17)
# Set a policy to use the newer MSVC ABI selection
cmake_policy(SET CMP0091 NEW)
# Selected the multithreaded runtime when using MSVC
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
project(jsonrpc LANGUAGES C CXX)
# Use C99
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
# Requires adherence to this standard
set(CMAKE_C_STANDARD_REQUIRED TRUE)
include(GenerateExportHeader)

add_library(jsonrpc SHARED jsonrpc.c )
# target_include_directories(.
#                            PUBLIC
#                            "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
#                            "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
# )
if(WIN32)
  target_link_libraries(jsonrpc PRIVATE wsock32 ws2_32)
endif()

find_package(json-c CONFIG REQUIRED)
target_link_libraries(jsonrpc PUBLIC json-c::json-c)

if(WIN32)
    find_package(PThreads4W REQUIRED)
    target_link_libraries(jsonrpc PRIVATE PThreads4W::PThreads4W)
endif()
target_include_directories(jsonrpc INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} )
generate_export_header(jsonrpc)
install(TARGETS jsonrpc)
