#include "func_sha.h"
#include <openssl/sha.h>
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