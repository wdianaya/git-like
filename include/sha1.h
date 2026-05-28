#ifndef SHA_UTILS
#define SHA_UTILS

#include <stddef.h>

// Вычислить SHA1 и вернуть hex-строку
void compute_sha1(const unsigned char *data, size_t len, char hash_hex[41]);

#endif