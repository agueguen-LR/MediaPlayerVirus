#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


/* Programme C pour effectuer un chiffrement/déchiffrement XOR */

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

    print_slow("              _,.---- , ----.,_\n", 93750);
    print_slow("          ,;~'                 '~;,\n", 93750);
    print_slow("        ,;                        ;,\n", 93750);
    print_slow("        ;                          ;\n", 93750);
    print_slow("       ,'                          ',\n", 93750);
    print_slow("      ,;                            ;,\n", 93750);
    print_slow("      ; ;       .           .       ; ;\n", 93750);
    print_slow("      | ;    ______       ______    ; |\n", 93750);
    print_slow("          '/~\"   ~\" . \"~   \"~\\'   \n", 93750);
    print_slow("         ~   ,-~~~^~, | ,~^~~~-,   ~   \n", 93750);
    print_slow("       |    |        }:{        |    |\n", 93750);
    print_slow("       |    l       / | \\      !    |\n", 93750);
    print_slow("       .~   (__,.--\".^.\"--.,__)   ~.\n", 93750);
    print_slow("       |     ----;'// | \\';----     |\n", 93750);
    print_slow("        \\__.      \\/^\\/      .__//\n", 93750);
    print_slow("         V| \\                 // |V\n", 93750);
    print_slow("          |  |T~\\___!___!___/~T| |\n", 93750);
    print_slow("          |  |` IIII_III_IIII '|  |\n", 93750);
    print_slow("          |   \\,II IIIII II,//   |\n", 93750);
    print_slow("           \\    `~~~~~~~~~~'   //\n", 93750);
    print_slow("              \\    .     .  //\n", 93750);
    print_slow("               \\.    ^   .//\n", 93750);
    print_slow("                  ^~~~^~~~^\n", 93750);
    print_slow("\n", 93750);

    printf("\033[0m");
}

/* XOR générique : travaille sur des ogctets, longueur explicite */
void xor_bytes(const unsigned char *input, size_t len,
               const char *key, unsigned char *output) {
    size_t key_len = strlen(key);
    for (size_t i = 0; i < len; i++) {
        output[i] = input[i] ^ (unsigned char)key[i % key_len];
    }
}

/*
 * Parse une chaîne hex de la forme "03 00 15 07 0a" (avec ou sans espaces).
 * Retourne le nombre d'octets écrits dans out, ou -1 en cas d'erreur.
 */
int hex_to_bytes(const char *hex_str, unsigned char *out, size_t out_size) {
    size_t count = 0;
    const char *p = hex_str;
    while (*p) {
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0') break;

        unsigned int byte;
        if (sscanf(p, "%2x", &byte) != 1) return -1;
        if (count >= out_size) return -1;
        out[count++] = (unsigned char)byte;
        p += 2;
    }
    return (int)count;
}

int main(void) {
    print_banner();
    printf("=== Programme XOR ===\n");
    printf("Mode ? [1] Chiffrer  [2] Déchiffrer : ");

    char mode_buf[8];
    fgets(mode_buf, sizeof(mode_buf), stdin);
    int mode = atoi(mode_buf);

    if (mode != 1 && mode != 2) {
        fprintf(stderr, "Mode invalide. Entrez 1 ou 2.\n");
        return EXIT_FAILURE;
    }

    char key[256];
    printf("Entrez la clé : ");
    fgets(key, sizeof(key), stdin);
    key[strcspn(key, "\n")] = '\0';

    if (mode == 1) {
        char input[256];
        printf("Entrez la chaîne à chiffrer : ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        size_t len = strlen(input);
        unsigned char output[256];
        xor_bytes((unsigned char *)input, len, key, output);

        printf("Résultat (hex) : ");
        for (size_t i = 0; i < len; i++) {
            printf("%02x ", output[i]);
        }
        printf("\n");

    } else {
        char hex_input[768];  /* 256 octets × 3 chars max */
        printf("Entrez la chaîne hex à déchiffrer (ex: 03 00 15 07 0a) : ");
        fgets(hex_input, sizeof(hex_input), stdin);
        hex_input[strcspn(hex_input, "\n")] = '\0';

        unsigned char bytes[256];
        int len = hex_to_bytes(hex_input, bytes, sizeof(bytes));
        if (len < 0) {
            fprintf(stderr, "Erreur : entrée hex invalide.\n");
            return EXIT_FAILURE;
        }

        unsigned char output[256];
        xor_bytes(bytes, (size_t)len, key, output);
        output[len] = '\0';

        printf("Résultat (texte) : %s\n", (char *)output);
    }

    return EXIT_SUCCESS;
}