cmake_minimum_required(VERSION 2.8.12)

project(glui LANGUAGES CXX)

SET(GLUI_SRCS
  glui_add_controls.cpp
  glui.cpp
  glui_bitmap_img_data.cpp
  glui_bitmaps.cpp
  glui_button.cpp
  glui_edittext.cpp
  glui_checkbox.cpp
  glui_node.cpp
  glui_radio.cpp
  glui_statictext.cpp
  glui_panel.cpp
  glui_separator.cpp
  glui_spinner.cpp
  glui_control.cpp
  glui_column.cpp
  glui_translation.cpp
  glui_rotation.cpp
  glui_mouse_iaction.cpp
  glui_listbox.cpp
  glui_rollout.cpp
  arcball.cpp
  algebra3.cpp
  quaternion.cpp
  glui.h
)
add_library(glui_static STATIC ${GLUI_SRCS})
target_link_options(glui_static PRIVATE -static-libgcc -static-libstdc++ -static)

target_include_directories(glui_static PRIVATE
    ../glui_gl
)
