cmake_minimum_required(VERSION 3.15)

if(POLICY CMP0110)
  cmake_policy(SET CMP0110 NEW)
endif()

project(MediaPlayer C)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)

add_subdirectory(src)
