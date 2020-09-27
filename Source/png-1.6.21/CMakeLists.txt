cmake_minimum_required(VERSION 2.8.12)

project(png LANGUAGES C)
# set(PROJECT_VERSION 2.0.15)


set(PNG_SRCS
png.c pngset.c pngget.c pngrutil.c pngtrans.c pngwutil.c
pngread.c pngrio.c pngwio.c pngwrite.c pngrtran.c
pngwtran.c pngmem.c pngerror.c pngpread.c
)


add_library(png STATIC ${PNG_SRCS})
target_include_directories(png PRIVATE ".")
# set(BUILD_SHARED_LIBS off)
# set(BUILD_STATIC_LIBS on)
# target_compile_options(gd PRIVATE -fPIC -m64 -I. -I../png-1.6.21 -I../PNG-9b -I../zlib128 -c -g  -DHAVE_LIBPNG -DHAVE_LIBZ -DHAVE_LIBPNG -DHAVE_LIBX11)
# SET(CMAKE_C_ARCHIVE_CREATE "<CMAKE_AR> rc <TARGET> <LINK_FLAGS> <OBJECTS>")
# target_link_libraries(glui_static
#   freeglut
#   glu32
# )

# target_compile_options(glui_static PRIVATE -lstdc++ -static-libgcc -static-libstdc++)
# target_link_options(glui_static PRIVATE -lstdc++ -static-libgcc -static-libstdc++)
# target_include_directories(glui_obj
#       PUBLIC
#       include
#       ${OPENGL_INCLUDE_DIR}
#       PRIVATE
#       ${GLUT_INCLUDE_DIR})
# # we need to enable -fPIC this so that the same object code can be used to
# # create static *and* shared libraries without double compilation
# set_target_properties( glui_obj PROPERTIES POSITION_INDEPENDENT_CODE 1)

# add_library(glui SHARED $<TARGET_OBJECTS:glui_obj>)
# target_include_directories(glui
#       PUBLIC
#       "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>"
#       "$<INSTALL_INTERFACE:include>"
#       ${OPENGL_INCLUDE_DIR}
#       PRIVATE
#       ${GLUT_INCLUDE_DIR})
# target_link_libraries(glui PUBLIC ${GLUT_LIBRARIES} ${OPENGL_LIBRARIES})
# set_target_properties(glui PROPERTIES
#   DEBUG_POSTFIX "d"
#   VERSION ${PROJECT_VERSION}
#   SOVERSION ${PROJECT_VERSION})

# add_library(glui_static STATIC $<TARGET_OBJECTS:glui_obj>)
# target_include_directories(glui_static
#       PUBLIC
#       "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>"
#       "$<INSTALL_INTERFACE:include>"
#       ${OPENGL_INCLUDE_DIR}
#       PRIVATE
#       ${GLUT_INCLUDE_DIR})
# target_link_libraries(glui_static PUBLIC ${GLUT_LIBRARIES} ${OPENGL_LIBRARIES})
# set_target_properties(glui_static PROPERTIES DEBUG_POSTFIX "d")


# ####
# # Installation

# set(config_install_dir "lib/cmake/${PROJECT_NAME}")
# set(include_install_dir "include")

# set(generated_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")

# # Configuration
# set(version_config "${generated_dir}/${PROJECT_NAME}ConfigVersion.cmake")
# set(project_config "${generated_dir}/${PROJECT_NAME}Config.cmake")
# set(targets_export_name "${PROJECT_NAME}Targets")
# set(namespace "${PROJECT_NAME}::")

# # Include module with fuction 'write_basic_package_version_file'
# include(CMakePackageConfigHelpers)

# # Configure '<PROJECT-NAME>ConfigVersion.cmake'
# write_basic_package_version_file(
#     "${version_config}" VERSION ${PROJECT_VERSION} COMPATIBILITY SameMajorVersion
# )

# # Configure '<PROJECT-NAME>Config.cmake'
# configure_package_config_file(
#     "cmake/Config.cmake.in"
#     "${project_config}"
#     INSTALL_DESTINATION "${config_install_dir}"
# )

# # Targets:
# install(
#     TARGETS glui_static glui
#     EXPORT "${targets_export_name}"
#     LIBRARY DESTINATION "lib"
#     ARCHIVE DESTINATION "lib"
#     RUNTIME DESTINATION "bin"
#     INCLUDES DESTINATION "${include_install_dir}"
# )

# # Headers:
# install(
#     DIRECTORY "${CMAKE_SOURCE_DIR}/include/"
#     DESTINATION "${include_install_dir}"
#     FILES_MATCHING PATTERN "*.h"
# )


# # Config
# install(
#     FILES "${project_config}" "${version_config}"
#     DESTINATION "${config_install_dir}"
# )

# # Config
# install(
#     EXPORT "${targets_export_name}"
#     NAMESPACE "${namespace}"
#     DESTINATION "${config_install_dir}"
# )
