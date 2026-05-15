#ifndef FUNC_SHA
#define FUNC_SHA
#include <stdlib.h>

// вычисляет SHA1 хеш от данных в буфере
// возвращает строку в hex формате (40 символов + \0)
void compute_sha1(const unsigned char *data, size_t len, char hash_hex[41]);

#endif