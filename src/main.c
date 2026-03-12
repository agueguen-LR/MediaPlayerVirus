#include <dirent.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main(void) {
  DIR *dir;
  struct dirent *dp;
  if ((dir = opendir(".")) == NULL) {
    perror("Cannot open .");
    exit(EXIT_FAILURE);
  }
  while ((dp = readdir(dir)) != NULL) {
    struct stat st;
    if (stat(dp->d_name, &st) == 0 && S_ISREG(st.st_mode) &&
        (st.st_mode & S_IXUSR) && strcmp(dp->d_name, "main")) {
      printf("Nom : %s, Mode: %o\n", dp->d_name, st.st_mode);
    }
  }
  closedir(dir);
  exit(EXIT_SUCCESS);
}

void fileFinder() {
  DIR *dir;
  struct dirent *dp;
  if ((dir = opendir(".")) == NULL) {
    perror("Cannot open .");
    exit(EXIT_FAILURE);
  }
  while ((dp = readdir(dir)) != NULL) {
  }
}
