cmake_minimum_required(VERSION 3.15)

if(POLICY CMP0110)
  cmake_policy(SET CMP0110 NEW)
endif()

project(MediaPlayer C)

add_subdirectory(src)
