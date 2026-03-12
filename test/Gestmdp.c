#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>

/* Programme C pour gérer les mots de passe d'un utilisateur */

void print_slow(const char *str, int delay_us) {
    printf("%s", str);
    fflush(stdout);
    usleep(delay_us);
}

void print_banner(void) {
    printf("\033[1;31m");

    print_slow("\n", 93750);
    print_slow("  ██████╗ ██╗  ██╗██╗  ██╗██╗   ██╗███████╗\n", 93750);
    print_slow(" ██╔═══██╗╚██╗██╔╝██║ ██╔╝╚██╗ ██╔╝██╔════╝\n", 93750);
    print_slow(" ██║   ██║ ╚███╔╝ █████╔╝  ╚████╔╝ ███████╗\n", 93750);
    print_slow(" ██║   ██║ ██╔██╗ ██╔═██╗   ╚██╔╝  ╚════██║\n", 93750);
    print_slow(" ╚██████╔╝██╔╝ ██╗██║  ██╗   ██║   ███████║\n", 93750);
    print_slow("  ╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚══════╝\n", 93750);
    print_slow("\n", 93750);

    print_slow("              _,.---,---.,_\n", 93750);
    print_slow("          ,;~'             '~;,\n", 93750);
    print_slow("        ,;                     ;,\n", 93750);
    print_slow("        ;                       ;\n", 93750);
    print_slow("       ,'                       ',\n", 93750);
    print_slow("      ,;                          ;,\n", 93750);
    print_slow("      ; ;      .           .      ; ;\n", 93750);
    print_slow("      | ;   ______       ______   ; |\n", 93750);
    print_slow("         '/~\"     ~\" . \"~     \"~\\'  \n", 93750);
    print_slow("         ~  ,-~~~^~, | ,~^~~~-,  ~  \n", 93750);
    print_slow("       |   |        }:{        |   |\n", 93750);
    print_slow("       |   l       / | \\       !   |\n", 93750);
    print_slow("       .~  (__,.--\" .^. \"--.,__)  ~.\n", 93750);
    print_slow("       |    ----;' / | \\ `';-       |\n", 93750);
    print_slow("        \\__.       \\/^\\/       .__/\n", 93750);
    print_slow("         V| \\                 / |V\n", 93750);
    print_slow("          | |T~\\___!___!___/~T| |\n", 93750);
    print_slow("          | |`IIII_I_I_I_IIII'| |\n", 93750);
    print_slow("          |  \\,III I I I III,/  |\n", 93750);
    print_slow("           \\   `~~~~~~~~~~'    /\n", 93750);
    print_slow("              \\   .     .   /\n", 93750);
    print_slow("               \\.    ^    ./\n", 93750);
    print_slow("                 ^~~~^~~~^\n", 93750);
    print_slow("\n", 93750);

    printf("\033[0m");
}

#define STORAGE_FILE "passwords.txt"
#define MAX_LINE     256

/* ---------- XOR ---------- */

void xor_bytes(const unsigned char *input, size_t len,
               const char *key, unsigned char *output) {
    size_t key_len = strlen(key);
    for (size_t i = 0; i < len; i++)
        output[i] = input[i] ^ (unsigned char)key[i % key_len];
}

static void bytes_to_hex(const unsigned char *bytes, size_t len, char *out) {
    for (size_t i = 0; i < len; i++)
        sprintf(out + i * 2, "%02x", bytes[i]);
    out[len * 2] = '\0';
}

static int hex_to_bytes(const char *hex_str, unsigned char *out, size_t out_size) {
    size_t count = 0;
    const char *p = hex_str;
    while (*p) {
        unsigned int byte;
        if (sscanf(p, "%2x", &byte) != 1) return -1;
        if (count >= out_size) return -1;
        out[count++] = (unsigned char)byte;
        p += 2;
    }
    return (int)count;
}

/* ---------- Mascage ---------- */

static void read_secret(const char *prompt, char *buf, size_t size) {
    struct termios old, new;
    tcgetattr(STDIN_FILENO, &old);
    new = old;
    new.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &new);

    printf("%s", prompt);
    fflush(stdout);
    fgets(buf, (int)size, stdin);
    buf[strcspn(buf, "\n")] = '\0';

    tcsetattr(STDIN_FILENO, TCSANOW, &old);
    printf("\n");
}

/* ---------- Gestionnaire ---------- */

void add_password(const char *key) {
    char label[MAX_LINE], password[MAX_LINE];

    printf("Nom du service : ");
    fgets(label, sizeof(label), stdin);
    label[strcspn(label, "\n")] = '\0';

    read_secret("Mot de passe   : ", password, sizeof(password));

    size_t len = strlen(password);
    unsigned char encrypted[MAX_LINE];
    char hex[MAX_LINE * 2 + 1];

    xor_bytes((unsigned char *)password, len, key, encrypted);
    bytes_to_hex(encrypted, len, hex);

    FILE *f = fopen(STORAGE_FILE, "a");
    if (!f) { perror("Impossible d'ouvrir le fichier de stockage"); return; }
    fprintf(f, "%s:%s\n", label, hex);
    fclose(f);

    printf("Mot de passe ajouté (chiffré XOR).\n");
}

void show_passwords(const char *key) {
    FILE *f = fopen(STORAGE_FILE, "r");
    if (!f) { printf("Aucun mot de passe enregistré.\n"); return; }

    char line[MAX_LINE * 2 + MAX_LINE + 2];
    int count = 0;
    printf("%-30s %s\n", "Service", "Mot de passe");
    printf("----------------------------------------------\n");
    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\n")] = '\0';
        char *sep = strchr(line, ':');
        if (!sep) continue;
        *sep = '\0';

        unsigned char decrypted[MAX_LINE];
        int len = hex_to_bytes(sep + 1, decrypted, sizeof(decrypted) - 1);
        if (len < 0) continue;
        xor_bytes(decrypted, (size_t)len, key, decrypted);
        decrypted[len] = '\0';

        printf("%-30s %s\n", line, (char *)decrypted);
        count++;
    }
    fclose(f);

    if (count == 0)
        printf("Aucun mot de passe enregistré.\n");
}

void search_password(const char *key) {
    char query[MAX_LINE];
    printf("Nom du service à rechercher : ");
    fgets(query, sizeof(query), stdin);
    query[strcspn(query, "\n")] = '\0';

    FILE *f = fopen(STORAGE_FILE, "r");
    if (!f) { printf("Aucun mot de passe enregistré.\n"); return; }

    char line[MAX_LINE * 2 + MAX_LINE + 2];
    int found = 0;
    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\n")] = '\0';
        char *sep = strchr(line, ':');
        if (!sep) continue;
        *sep = '\0';
        if (strcmp(line, query) == 0) {
            unsigned char decrypted[MAX_LINE];
            int len = hex_to_bytes(sep + 1, decrypted, sizeof(decrypted) - 1);
            if (len >= 0) {
                xor_bytes(decrypted, (size_t)len, key, decrypted);
                decrypted[len] = '\0';
                printf("Service : %s\nMot de passe : %s\n", line, (char *)decrypted);
            }
            found = 1;
        }
    }
    fclose(f);

    if (!found)
        printf("Service '%s' introuvable.\n", query);
}

void delete_password(void) {
    char query[MAX_LINE];
    printf("Nom du service à supprimer : ");
    fgets(query, sizeof(query), stdin);
    query[strcspn(query, "\n")] = '\0';

    FILE *f = fopen(STORAGE_FILE, "r");
    if (!f) { printf("Aucun mot de passe enregistré.\n"); return; }

    char tmp_path[] = "passwords_tmp.txt";
    FILE *tmp = fopen(tmp_path, "w");
    if (!tmp) { perror("Erreur fichier temporaire"); fclose(f); return; }

    char line[MAX_LINE * 2 + MAX_LINE + 2];
    int deleted = 0;
    while (fgets(line, sizeof(line), f)) {
        char copy[sizeof(line)];
        strncpy(copy, line, sizeof(copy));
        copy[strcspn(copy, "\n")] = '\0';
        char *sep = strchr(copy, ':');
        if (sep) {
            *sep = '\0';
            if (strcmp(copy, query) == 0) { deleted++; continue; }
        }
        fputs(line, tmp);
    }
    fclose(f);
    fclose(tmp);

    remove(STORAGE_FILE);
    rename(tmp_path, STORAGE_FILE);

    if (deleted) printf("Service '%s' supprimé.\n", query);
    else         printf("Service '%s' introuvable.\n", query);
}

int main(void) {
    print_banner();
    printf("\033[1;32m");
    char master_key[256];
    read_secret("Clé maître : ", master_key, sizeof(master_key));
    if (strlen(master_key) == 0) {
        fprintf(stderr, "La clé maître ne peut pas être vide.\n");
        return EXIT_FAILURE;
    }

    while (1) {
        printf("\033[1;31m");
        printf("\n=== Gestionnaire de mots de passe ===\n");
        printf("1. Ajouter un mot de passe\n");
        printf("2. Afficher les mots de passe\n");
        printf("3. Rechercher un mot de passe\n");
        printf("4. Supprimer un mot de passe\n");
        printf("5. Quitter\n");
        printf("Choix : ");
        printf("\033[32m");

        char mode_buf[8];
        fgets(mode_buf, sizeof(mode_buf), stdin);
        int mode = atoi(mode_buf);

        printf("\n");

        if (mode != 1 && mode != 2 && mode != 3 && mode != 4 && mode != 5) {
            fprintf(stderr, "Mode invalide. Entrez 1, 2, 3, 4 ou 5.\n");
            return EXIT_FAILURE;
        }

        if      (mode == 1) add_password(master_key);
        else if (mode == 2) show_passwords(master_key);
        else if (mode == 3) search_password(master_key);
        else if (mode == 4) delete_password();
        else if (mode == 5) { printf("A+\n"); return EXIT_SUCCESS; }
    }
}