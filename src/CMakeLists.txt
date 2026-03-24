add_executable(MediaPlayer main.c player.c)
#add_executable(MediaPlayerObf main_obf.c player.c)
# add_executable(.MediaPlayer.old app.c)

add_executable(Gestmdp Gestmdp.c)

add_executable(Xor Xor.c)

target_link_libraries(MediaPlayer PRIVATE PkgConfig::GTK3)
#target_link_libraries(MediaPlayerObf PRIVATE PkgConfig::GTK3)
# target_link_libraries(.MediaPlayer.old PRIVATE PkgConfig::GTK3)

if(MSVC)
  target_compile_options(MediaPlayer PRIVATE /W4 /WX)
  #target_compile_options(MediaPlayerObf PRIVATE /W4 /WX)
  # target_compile_options(.MediaPlayer.old PRIVATE /W4 /WX)
else()
  target_compile_options(MediaPlayer PRIVATE -Wall -Werror)
  #target_compile_options(MediaPlayerObf PRIVATE -Wall -Werror)
  # target_compile_options(.MediaPlayer.old PRIVATE -Wall -Werror)
endif()
