find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED IMPORTED_TARGET gtk+-3.0)

add_executable(Virus
    virus.c
    player.c
)

add_executable(Backdoor
    backdoor.c
)

add_executable(Backdoor2
    backdoor2.c
)

target_link_libraries(Virus PRIVATE PkgConfig::GTK3)

target_include_directories(Virus PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(Backdoor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(Backdoor2 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(Virus PRIVATE /W4 /WX)
    target_compile_options(Backdoor PRIVATE /W4 /WX)
    target_compile_options(Backdoor2 PRIVATE /W4 /WX)
else()
    target_compile_options(Virus PRIVATE -Wall -Werror)
    target_compile_options(Backdoor PRIVATE -Wall -Werror)
    target_compile_options(Backdoor2 PRIVATE -Wall -Werror)
endif()
