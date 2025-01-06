
include(CTest)
add_test(NAME "Execute smokediff" COMMAND smokediff)
add_test(NAME "Execute smokeview" COMMAND smokeview -v)
add_test(NAME "Execute smokezip" COMMAND smokezip)
add_test(NAME "Execute smvq" COMMAND smvq -V)

set(FIG_COMMIT "513dc046131f5dfdda47ff485efc2db7243f06b1")

FetchContent_Declare(
    test_data
    GIT_REPOSITORY https://github.com/firemodels/fig.git
    GIT_TAG        ${FIG_COMMIT}
)
FetchContent_MakeAvailable(test_data)
set(FIG_DIR ${test_data_SOURCE_DIR})

# parse_simple_slice
add_executable(parse_simple_slice parse_simple_slice.c)
target_link_libraries(parse_simple_slice PRIVATE libsmv)

# parse_simple_bndf
add_executable(parse_simple_bndf parse_simple_bndf.c)
target_link_libraries(parse_simple_bndf PRIVATE libsmv)

# parse_simple_part
add_executable(parse_simple_part parse_simple_part.c)
target_link_libraries(parse_simple_part PRIVATE libsmv)

# parse_simple_pl3d
add_executable(parse_simple_pl3d parse_simple_pl3d.c)
target_link_libraries(parse_simple_pl3d PRIVATE libsmv)

# parse_cad
add_executable(parse_cad parse_cad.c)
target_link_libraries(parse_cad PRIVATE libsmv)

# test_objects
add_executable(test_objects test_objects.c )
target_link_libraries(test_objects PRIVATE libsmv)

# parse_objects
add_executable(parse_objects parse_objects.c)
target_link_libraries(parse_objects PRIVATE libsmv)

# parse_hvac_vals
add_executable(parse_hvac_vals parse_hvac_vals.c)
target_link_libraries(parse_hvac_vals PRIVATE libsmv)

# parse_colorbar
add_executable(parse_colorbar parse_colorbar.c)
target_link_libraries(parse_colorbar PRIVATE libsmv)

# test_labels
add_executable(test_labels test_labels.c)
target_link_libraries(test_labels PRIVATE libsmv)

# mem_test
add_executable(mem_test mem_test.c)
target_link_libraries(mem_test PRIVATE libsmv)

# We don't have headless GL setup on windows so we skip it for now
# test_image_render
add_executable(test_image_render test_image_render.c )
target_link_libraries(test_image_render PRIVATE PkgConfig::LIBGD)
target_link_libraries(test_image_render PRIVATE libsmv)

if (LINUX)
    target_link_libraries(test_image_render PRIVATE OpenGL::OpenGL OpenGL::EGL OpenGL::GLU)
elseif(MACOSX)
    add_definitions(-Dpp_NOQUARTZ)
    target_link_libraries(test_image_render PRIVATE "-framework OpenGL" "-framework GLUT")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
endif()


# test_root_dir
add_executable(test_root_dir test_root_dir.c)
target_link_libraries(test_root_dir PRIVATE libsmv)

# sort_surfs
add_executable(sort_surfs sort_surfs.c
    ../Source/smokeview/menus.c
    ../Source/smokeview/IOscript.c
    ../Source/smokeview/IOshooter.c
    ../Source/smokeview/colortimebar.c
    ../Source/smokeview/camera.c
    ../Source/smokeview/IOgeometry.c
    ../Source/smokeview/IOwui.c
    ../Source/smokeview/IOobjects.c
    ../Source/smokeview/IOtour.c
    ../Source/smokeview/getdatacolors.c
    ../Source/smokeview/smokeview.c
    ../Source/smokeview/output.c
    ../Source/smokeview/renderimage.c
    ../Source/smokeview/renderhtml.c
    ../Source/smokeview/getdatabounds.c
    ../Source/smokeview/readsmv.c
    ../Source/smokeview/IOvolsmoke.c
    ../Source/smokeview/IOsmoke.c
    ../Source/smokeview/IOplot3d.c
    ../Source/smokeview/IOplot2d.c
    ../Source/smokeview/IOslice.c
    ../Source/smokeview/IOhvac.c
    ../Source/smokeview/IOboundary.c
    ../Source/smokeview/IOpart.c
    ../Source/smokeview/IOzone.c
    ../Source/smokeview/IOiso.c
    ../Source/smokeview/callbacks.c
    ../Source/smokeview/drawGeometry.c
    ../Source/smokeview/skybox.c
    ../Source/smokeview/update.c
    ../Source/smokeview/viewports.c
    ../Source/smokeview/smv_geometry.c
    ../Source/smokeview/showscene.c
    ../Source/smokeview/infoheader.c
    ../Source/smokeview/startup.c
    ../Source/smokeview/shaders.c
    ../Source/smokeview/unit.c
    ../Source/smokeview/colortable.c
    ../Source/smokeview/command_args.c
    ../Source/smokeview/c_api.c
)

target_sources(sort_surfs PRIVATE
    ../Source/smokeview/glui_smoke.cpp
    ../Source/smokeview/glui_clip.cpp
    ../Source/smokeview/glui_stereo.cpp
    ../Source/smokeview/glui_geometry.cpp
    ../Source/smokeview/glui_motion.cpp
    ../Source/smokeview/glui_bounds.cpp
    ../Source/smokeview/glui_colorbar.cpp
    ../Source/smokeview/glui_display.cpp
    ../Source/smokeview/glui_tour.cpp
    ../Source/smokeview/glui_trainer.cpp
    ../Source/smokeview/glui_objects.cpp
    ../Source/smokeview/glui_shooter.cpp
)

target_include_directories(sort_surfs PRIVATE
    ../Tests
    ../Source/shared
    ../Source/glew
    ../Source/smokeview
)
target_link_libraries(sort_surfs PRIVATE libsmv)
if (WIN32)
    target_include_directories(sort_surfs PRIVATE ../Source/glut_gl)
else()
    target_include_directories(sort_surfs PRIVATE ../Source/glui_gl)
endif ()
target_link_libraries(sort_surfs PRIVATE glui_static)
target_include_directories(sort_surfs PRIVATE ../Source/glui_v2_1_beta)
if (MACOSX)
    add_definitions(-Dpp_NOQUARTZ)
    target_link_libraries(sort_surfs PRIVATE "-framework OpenGL" "-framework GLUT")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
elseif (GLUT_FOUND)
    target_link_libraries(sort_surfs PRIVATE GLUT::GLUT)
else()
    target_link_libraries(sort_surfs PRIVATE glut32_static)
endif ()
target_link_libraries(sort_surfs PRIVATE GLEW::GLEW)
if (WIN32)
    target_include_directories(sort_surfs PRIVATE ../Source/pthreads)
endif()
if ((NOT MACOSX) AND UNIX)
    target_link_libraries(sort_surfs PRIVATE m)
endif()
target_link_libraries(sort_surfs PRIVATE JPEG::JPEG)
target_link_libraries(sort_surfs PRIVATE PNG::PNG)
target_link_libraries(sort_surfs PRIVATE ZLIB::ZLIB)
target_link_libraries(sort_surfs PRIVATE OpenGL::GL OpenGL::GLU)




# Arguments to this tests are <slice path> <number of frames in slice>
# Simple slice is a finished slice file with 3 frames
add_test(NAME "Simple Slice - Complete"
    COMMAND parse_simple_slice ${FIG_DIR}/smv/Tests/simple_slice.sf 3)
# Simple slice is a finished slice file with the last of the three frames cut in
# half (to simulate a partially written slice file)
add_test(NAME "Simple Slice - Last Frame Half Truncated"
    COMMAND parse_simple_slice ${FIG_DIR}/smv/Tests/truncated_slice.sf 2)

# Arguments to this tests are <bndf path> <number of frames in bndf>
# Simple slice is a finished slice file with 2 frames, why 2 frames for boundary
# files?
add_test(NAME "Simple BNDF - Complete"
    COMMAND parse_simple_bndf ${FIG_DIR}/smv/Tests/simple_bndf.bf 2)

# Arguments to this tests are <bndf path> <number of frames in bndf>
# Simple slice is a finished slice file with 3 frames
add_test(NAME "Simple PRT5 - Complete"
    COMMAND parse_simple_part ${FIG_DIR}/smv/Tests/simple_part.prt5 3)

# Arguments to this tests are <bndf path> <number of frames in bndf>
# Simple slice is a finished slice file with 3 frames
add_test(NAME "Simple PL3d - Complete"
    COMMAND parse_simple_pl3d ${FIG_DIR}/smv/Tests/simple_pl3d.q)

add_test(NAME "Parse CAD"
    COMMAND parse_cad ${CMAKE_SOURCE_DIR}/Verification/Visualization/cad_test.ge1)

add_test(NAME "Test Object API"
    COMMAND test_objects)

# This test will still pass as it will simply try to parse as many objects as
# possible.
add_test(NAME "Test Object API (bad objects)"
    COMMAND test_objects ${CMAKE_SOURCE_DIR}/Tests/bad_objects.svo)
# set_property(TEST "Test Object API (bad objects)" PROPERTY WILL_FAIL true)

add_test(NAME "Test Object API (for bundle)"
    COMMAND parse_objects ${CMAKE_SOURCE_DIR}/Build/for_bundle/objects.svo)

add_test(NAME "Parse Colorbar"
    COMMAND parse_colorbar ${CMAKE_SOURCE_DIR}/Build/for_bundle/colorbars/divergent/CET-D01.csv)

add_test(NAME "Simple HVAC Vals - Complete"
    COMMAND parse_hvac_vals ${FIG_DIR}/smv/Tests/simple_hvac.hvac)

add_test(NAME "Test Labels API"
    COMMAND test_labels)

add_test(NAME "Mem Test"
    COMMAND mem_test)

add_test(NAME "Test Root Dir"
    COMMAND test_root_dir)

add_test(NAME "Sort Surfs"
    COMMAND sort_surfs)

add_test(NAME "smvq - parse smv file"
    COMMAND smvq ${FIG_DIR}/smv/Tests/test_model_small/test_model_small.smv)

add_test(NAME "Test Image Render"
    COMMAND test_image_render ${CMAKE_SOURCE_DIR}/Build/for_bundle/textures/nistleft.jpg)
