project('Smokeview', ['c','cpp','fortran'])
incdir = include_directories(['Source/smokeview','Source/shared',
    'Source/glui_gl','Source/glfw/include','Source/glew',
    'Source/glui_v2_1_beta','Source/gd-2.0.15','Source/libui'])
src = ['Source/smokeview/main.c', 'Source/smokeview/menus.c', 'Source/smokeview/IOscript.c',
    'Source/smokeview/IOshooter.c', 'Source/smokeview/glui_objects.cpp', 'Source/smokeview/glui_shooter.cpp', 'Source/smokeview/glui_wui.cpp',
    'Source/shared/csphere.c', 'Source/smokeview/colortimebar.c', 'Source/shared/smv_endian.c', 'Source/smokeview/camera.c', 'Source/smokeview/IOgeometry.c', 'Source/smokeview/IOwui.c',
    'Source/smokeview/IOobjects.c', 'Source/smokeview/IOtour.c', 'Source/smokeview/glui_display.cpp', 'Source/smokeview/glui_tour.cpp', 'Source/smokeview/getdatacolors.c', 'Source/smokeview/smokeview.c',
    'Source/smokeview/output.c', 'Source/smokeview/renderimage.c', 'Source/smokeview/renderhtml.c', 'Source/shared/isobox.c', 'Source/smokeview/getdatabounds.c', 'Source/smokeview/readsmv.c', 'Source/smokeview/scontour2d.c',
    'Source/smokeview/glui_smoke.cpp', 'Source/smokeview/glui_clip.cpp', 'Source/smokeview/glui_stereo.cpp', 'Source/smokeview/glui_geometry.cpp', 'Source/smokeview/glui_motion.cpp',
    'Source/smokeview/glui_bounds.cpp', 'Source/shared/dmalloc.c', 'Source/shared/assert.c', 'Source/shared/stdio_m.c',
    'Source/shared/compress.c', 'Source/smokeview/IOvolsmoke.c', 'Source/smokeview/IOsmoke.c', 'Source/smokeview/IOplot3d.c', 'Source/smokeview/IOslice.c', 'Source/smokeview/IOboundary.c',
    'Source/smokeview/IOpart.c', 'Source/smokeview/IOzone.c', 'Source/smokeview/IOiso.c', 'Source/smokeview/callbacks.c', 'Source/smokeview/drawGeometry.c',
    'Source/smokeview/glui_colorbar.cpp', 'Source/smokeview/skybox.c', 'Source/shared/file_util.c', 'Source/shared/string_util.c', 'Source/smokeview/startup.c', 'Source/smokeview/glui_trainer.cpp',
    'Source/smokeview/shaders.c', 'Source/smokeview/unit.c', 'Source/smokeview/threader.c', 'Source/shared/histogram.c', 'Source/shared/translate.c', 'Source/smokeview/update.c', 'Source/smokeview/viewports.c',
    'Source/smokeview/smv_geometry.c', 'Source/smokeview/showscene.c', 'Source/smokeview/infoheader.c',  'Source/shared/md5.c', 'Source/shared/sha1.c', 'Source/shared/sha256.c',
    # 'Source/smokeview/fdsmodules.f90', 'Source/smokeview/gsmv.f90', 'Source/smokeview/getdata.f90',
    'Source/glew/glew.c']
# add_global_arguments('-Dpp_FSEEK', language : 'fortran')
gl_dep = dependency('GL')
libui = dependency('libui')
# warning_level = 3
# libui = subproject('libui', default_options: ['warning_level=3','b_pch=false'])
# glu_dep = dependency('GLU32')
# glui_dep = dependency('glui', method : 'cmake')
# inc = include_directories('include')
# libsimple = shared_library('simple',
#   'simple.c',
#   include_directories : inc,
#   install : true)

# libsimple_dep = declare_dependency(include_directories : [],
#   link_with : libsimple)

executable('smokeview', sources : src, include_directories : incdir,
    fortran_args : ['-cpp'], dependencies : [gl_dep, libui],
    default_options: ['b_pch=false','warning_level=3'])
