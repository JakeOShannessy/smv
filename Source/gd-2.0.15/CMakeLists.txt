cmake_minimum_required(VERSION 2.8.12)

project(gd LANGUAGES C)
set(PROJECT_VERSION 2.0.15)

set(GD_SRCS
  gd.c gd_gd.c gd_gd2.c gd_io.c gd_io_dp.c
  gd_io_file.c gd_ss.c gd_io_ss.c gd_png.c gd_jpeg.c gdxpm.c
  gdfontt.c gdfonts.c gdfontmb.c gdfontl.c gdfontg.c
  gdtables.c gdft.c gdcache.c gdkanji.c wbmp.c
  gd_wbmp.c gdhelpers.c gd_topal.c
)

add_library(gd STATIC ${GD_SRCS})

target_include_directories(gd PRIVATE . ../jpeg-9b ../png-1.6.21 ../zlib128)
add_definitions(-DHAVE_LIBPNG -DHAVE_LIBZ -DHAVE_LIBJPEG -DHAVE_LIBX11)
target_compile_options(gd PRIVATE -fPIC -m64)
target_link_options(gd PRIVATE -static-libgcc -static-libstdc++ -static)
