add_executable(Gestmdp Gestmdp.c)
add_executable(Xor Xor.c)

target_link_libraries(app PRIVATE PkgConfig::GTK4)

if(MSVC)
  target_compile_options(main PRIVATE /W4 /WX)
else()
	target_compile_options(main PRIVATE -Wall -Werror)
endif()
