#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int lancer_ssh_serveur_2222() {
  pid_t pid = fork();

  if (pid < 0) {
    perror("fork failed");
    return -1;
  }

  if (pid == 0) {
    // Processus enfant : lance sshd sur port 2222
    char *args[] = {"sshd",
                    "-D", // Mode démon (ne se détache pas)
                    "-p",
                    "2222", // Port 2222
                    "-o",
                    "UsePrivilegeSeparation=no", // Pas de séparation privilèges
                                                 // (évite sudo)
                    "-o",
                    "PasswordAuthentication=yes", // Auth par mot de passe
                    "-o",
                    "PermitRootLogin=yes", // Root autorisé si besoin
                    NULL};

    execvp("sshd", args);
    perror("execvp sshd failed");
    exit(1);
  } else {
    // Processus parent : attend un peu et retourne le PID
    sleep(1);
    printf("Serveur SSH lancé sur port 2222 (PID: %d)\n", pid);
    printf("Testez avec: ssh user@localhost -p 2222\n");
    return pid;
  }
}

int main() {
  printf("Lancement serveur SSH port 2222...\n");

  lancer_ssh_serveur_2222();

  return 0;
}
