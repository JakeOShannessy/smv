cmake_minimum_required(VERSION 3.10)
project(Smokeview_fortw LANGUAGES Fortran)
add_library(smv_fortw SHARED fdsmodules.f90 gsmv.f90 getdata.f90)

if (UNIX)
    add_definitions(pp_LINUX)
endif ()
add_definitions(-Dpp_FSEEK)

set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -static-libgfortran -static")
