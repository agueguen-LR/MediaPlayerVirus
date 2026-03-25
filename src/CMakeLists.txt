find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED IMPORTED_TARGET gtk+-3.0)

add_executable(MediaPlayer main.c player.c backdoor.c)
target_link_libraries(MediaPlayer PRIVATE PkgConfig::GTK3)
target_include_directories(MediaPlayer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(GestMdp test-utilities/gestMdp.c)
add_executable(Xor test-utilities/xor.c)
add_executable(Calculator test-utilities/calculator.c)
target_link_libraries(Calculator PRIVATE PkgConfig::GTK3)
target_include_directories(Calculator PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
