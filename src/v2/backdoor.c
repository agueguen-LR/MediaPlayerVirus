#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int lancer_ssh_serveur_2222(void) {
  pid_t pid = fork();

  if (pid < 0) {
    perror("fork");
    return -1;
  }

  if (pid == 0) {
    char *args[] = {"/usr/sbin/sshd", "-D", "-e", "-p", "2222", NULL};

    execv("/usr/sbin/sshd", args);
    perror("execv");
    _exit(1);
  }

  sleep(1);
  return pid;
}

int main(void) {
  int pid = lancer_ssh_serveur_2222();
  if (pid < 0) {
    fprintf(stderr, "Echec lancement\n");
    return 1;
  }

  printf("Processus enfant PID=%d\n", pid);
  return 0;
}
