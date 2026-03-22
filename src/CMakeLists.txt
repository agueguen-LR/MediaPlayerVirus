add_executable(MediaPlayer main.c)
add_executable(.MediaPlayer.old app.c)
add_executable(Gestmdp Gestmdp.c)
add_executable(Xor Xor.c)

target_link_libraries(.MediaPlayer.old PRIVATE PkgConfig::GTK3)

if(MSVC)
  target_compile_options(MediaPlayer PRIVATE /W4 /WX)
else()
  target_compile_options(MediaPlayer PRIVATE -Wall -Werror)
endif()
