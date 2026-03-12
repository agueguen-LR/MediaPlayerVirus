add_executable(main main.c)
add_executable(app app.c)

target_link_libraries(app PRIVATE PkgConfig::GTK3)

if(MSVC)
  target_compile_options(main PRIVATE /W4 /WX)
else()
	target_compile_options(main PRIVATE -Wall -Werror)
endif()
