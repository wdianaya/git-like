#ifndef SHA_UTILS
#define SHA_UTILS

#include <stddef.h>
#include <openssl/sha.h>
#define SHA1_HEX_LEN 41

// Обычное хеширование (буфер целиком в памяти)
void compute_sha1(const unsigned char *data, size_t len, char hash_hex[SHA1_HEX_LEN]);

// Потоковое хеширование
void sha1_ctx_init(SHA_CTX *ctx);
void sha1_ctx_update(SHA_CTX *ctx, const unsigned char *data, size_t len);
void sha1_ctx_final(SHA_CTX *ctx, char hash_hex[SHA1_HEX_LEN]);

#endif