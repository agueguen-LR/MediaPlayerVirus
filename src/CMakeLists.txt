add_executable(main main.c)

if(MSVC)
  target_compile_options(main PRIVATE /W4 /WX)
else()
	target_compile_options(main PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
