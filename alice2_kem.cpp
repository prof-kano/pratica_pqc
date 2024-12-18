#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "oqs/oqs.h"

void write_to_file(const char *filename, const uint8_t *data, size_t length) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Erro ao abrir arquivo");
        exit(EXIT_FAILURE);
    }
    fwrite(data, 1, length, file);
    fclose(file);
}

uint8_t *read_from_file(const char *filename, size_t *length) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Erro ao abrir arquivo");
        exit(EXIT_FAILURE);
    }
    fseek(file, 0, SEEK_END);
    *length = ftell(file);
    fseek(file, 0, SEEK_SET);

    uint8_t *data = (uint8_t *)malloc(*length);
    if (fread(data, 1, *length, file) != *length) {
        perror("Erro ao ler arquivo");
        fclose(file);
        free(data);
        exit(EXIT_FAILURE);
    }
    fclose(file);
    return data;
}

void print_hex(const char *label, const uint8_t *data, size_t length) {
    printf("%s: ", label);
    for (size_t i = 0; i < length; i++) {
        printf("%02X", data[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n         ");
        }
    }
    printf("\n");
}

int main() {
    printf("=== Alice: Desencapsulamento do Segredo ===\n");

    const char *kem_alg = OQS_KEM_alg_ml_kem_512;
    printf("[INFO] Usando o algoritmo: %s\n", kem_alg);

    OQS_KEM *kem = OQS_KEM_new(kem_alg);
    if (!kem) {
        fprintf(stderr, "Erro ao inicializar o KEM.\n");
        return EXIT_FAILURE;
    }

    size_t secret_key_len;
    uint8_t *secret_key = read_from_file("secret_key.bin", &secret_key_len);

    if (secret_key_len != kem->length_secret_key) {
        fprintf(stderr, "Erro: Tamanho da chave secreta inválido.\n");
        return EXIT_FAILURE;
    }

    size_t ciphertext_len;
    uint8_t *ciphertext = read_from_file("ciphertext.bin", &ciphertext_len);

    if (ciphertext_len != kem->length_ciphertext) {
        fprintf(stderr, "Erro: Tamanho do texto cifrado inválido.\n");
        return EXIT_FAILURE;
    }

    uint8_t *shared_secret = (uint8_t *)malloc(kem->length_shared_secret);

    if (OQS_KEM_decaps(kem, shared_secret, ciphertext, secret_key) != OQS_SUCCESS) {
        fprintf(stderr, "Erro ao desencapsular o segredo.\n");
        return EXIT_FAILURE;
    }

    write_to_file("shared_secret_alice.bin", shared_secret, kem->length_shared_secret);

    printf("[INFO] Segredo compartilhado desencapsulado:\n");
    print_hex("Segredo compartilhado (Alice)", shared_secret, kem->length_shared_secret);

    printf("[INFO] Arquivo gerado:\n");
    printf("  - shared_secret_alice.bin\n");

    free(secret_key);
    free(ciphertext);
    free(shared_secret);
    OQS_KEM_free(kem);

    return EXIT_SUCCESS;
}
