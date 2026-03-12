cmake_minimum_required(VERSION 3.15)

if(POLICY CMP0110)
  cmake_policy(SET CMP0110 NEW)
endif()

project(MediaPlayer C)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED IMPORTED_TARGET gtk+-3.0)

add_subdirectory(src)
add_subdirectory(test)
