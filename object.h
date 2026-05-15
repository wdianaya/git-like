#ifndef OBJECTS_H
#define OBJECTS_H

#include <stddef.h>

// Вычислить SHA1 и вернуть hex-строку
void compute_sha1(const unsigned char *data, size_t len, char hash_hex[41]);

// Создать blob из файла, вернуть хеш
char* create_blob(const char *file_path);

#endif