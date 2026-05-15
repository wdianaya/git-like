#ifndef FUNC_SHA
#define FUNC_SHA
#include <stdlib.h>
#include <openssl/sha.h>

// вычисляет SHA1 хеш от данных в буфере
// возвращает строку в hex формате (40 символов + \0)
void comput_sha(const unsigned char *data, size_t len, char hash_hex[41]);

#endif