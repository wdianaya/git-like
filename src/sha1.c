#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <openssl/sha.h>

#include "sha1.h"

#define LEN_SHA 20

void compute_sha1(const unsigned char *data, size_t len, char hash_hex[41]) {
    unsigned char hash[LEN_SHA];

    SHA1(data, len, hash);
    // преобразуем бинарный хеш в hex строку
    for (int i = 0; i < LEN_SHA; i++) {
        sprintf(hash_hex + (i * 2), "%02x", hash[i]);
    }
    hash_hex[40] = '\0';
}

// Инициализация контекста для потокового хеширования
void sha1_ctx_init(SHA_CTX *ctx) {
    SHA1_Init(ctx);
    
}

// Добавление порции данных
void sha1_ctx_update(SHA_CTX *ctx, const unsigned char *data, size_t len) {
    SHA1_Update(ctx, data, len);
}

// Завершение хеширования и получение hex строки
void sha1_ctx_final(SHA_CTX *ctx, char hash_hex[41]) {
    unsigned char hash[LEN_SHA];
    SHA1_Final(hash, ctx);
    
    for (int i = 0; i < LEN_SHA; i++) {
        sprintf(hash_hex + (i * 2), "%02x", hash[i]);
    }
    hash_hex[40] = '\0';
}