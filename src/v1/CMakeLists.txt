find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED IMPORTED_TARGET gtk+-3.0)

add_executable(MediaPlayer main.c player.c)

add_executable(Gestmdp Gestmdp.c)

add_executable(Xor Xor.c)

target_link_libraries(MediaPlayer PRIVATE PkgConfig::GTK3)
target_include_directories(MediaPlayer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(MediaPlayer PRIVATE /W4 /WX)
else()
    target_compile_options(MediaPlayer PRIVATE -Wall -Werror)
endif()
