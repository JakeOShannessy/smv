cmake_minimum_required(VERSION 2.8.12)

project(winglut LANGUAGES C)

if (NOT UNIX)
  set(GLUT_SRCS
    # use (for now) for the PC
    glut_8x13.c glut_9x15.c glut_bitmap.c glut_bwidth.c glut_cindex.c glut_cmap.c glut_cursor.c glut_dials.c glut_dstr.c glut_event.c
    glut_ext.c glut_fullscrn.c glut_gamemode.c glut_get.c glut_glxext.c glut_hel10.c glut_hel12.c glut_hel18.c glut_init.c  glut_input.c
    glut_joy.c glut_key.c glut_keyctrl.c glut_keyup.c glut_modifier.c glut_mroman.c glut_overlay.c glut_roman.c
    glut_shapes.c glut_space.c glut_stroke.c   glut_swap.c glut_swidth.c glut_tablet.c glut_teapot.c glut_tr10.c glut_tr24.c glut_util.c
    glut_win.c glut_winmisc.c glut_mesa.c glut_warp.c glut_vidresize.c win32_glx.c win32_menu.c win32_util.c win32_x11.c win32_winproc.c
  )
else ()
  message(FATAL_ERROR "end")
  set(GLUT_SRCS
    glut_8x13.c glut_9x15.c glut_bitmap.c glut_bwidth.c glut_cindex.c glut_cmap.c glut_cursor.c glut_dials.c glut_dstr.c glut_event.c
    glut_ext.c glut_fullscrn.c glut_gamemode.c glut_get.c glut_glxext.c glut_hel10.c glut_hel12.c glut_hel18.c glut_init.c  glut_input.c
    glut_joy.c glut_key.c glut_keyctrl.c glut_keyup.c glut_menu.c glut_menu2.c glut_modifier.c glut_mroman.c glut_overlay.c glut_roman.c
    glut_shapes.c glut_space.c glut_stroke.c   glut_swap.c glut_swidth.c glut_tablet.c glut_teapot.c glut_tr10.c glut_tr24.c glut_util.c
    glut_win.c glut_winmisc.c layerutil.c glut_mesa.c glut_warp.c glut_vidresize.c
  )
endif ()
add_library(winglut STATIC ${GLUT_SRCS})

add_definitions(-D__MINGW32__)
add_definitions(-DWIN32)
add_definitions(-D_WIN32)
target_include_directories(winglut PUBLIC
  "../glut_gl"
)
target_link_libraries(winglut
  gdi32
  glu32
  winmm
  opengl32
)
target_compile_options(winglut PRIVATE -Wno-write-strings)
target_link_options(winglut PRIVATE -static-libgcc -static-libstdc++ -static)
