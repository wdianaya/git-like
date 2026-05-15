#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <openssl/sha.h>

#include "utils.h"

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

// read the file content
// store the blob object in database (.mygit/objects)
char* create_blob(const char* file_path) {
    FILE *f = fopen(file_path, "rb");
    if (!f) {
        printf("Cannot open file %s\n", file_path);
        return NULL;
    }

    // получаем размер файла в байтах
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    // выделяем память для чтения содержимого файла
    unsigned char *buffer = malloc(file_size);
    if (!buffer) {
        printf("mem error :(");
        fclose(f);
        return NULL;
    }

    size_t bytes_read = fread(buffer, sizeof(char), file_size, f);
    fclose(f);

    if (bytes_read != file_size) {
        printf("read error :(\n");
        free(buffer);
        return NULL;
    }

    char *hash_hex = malloc(41);
    compute_sha1(buffer, bytes_read, hash_hex);

    char dir_path[256];
    snprintf(dir_path, sizeof(dir_path), ".mygit/objects/%.2s", hash_hex);

    // проверка на существование данной директории
    if (!directory_exists(dir_path)) { 
        mkdir(dir_path, 0755);
    }

    char obj_path[512];
    snprintf(obj_path, sizeof(obj_path), ".mygit/objects/%.2s/%s", hash_hex, hash_hex + 2);
    if (file_exists(obj_path)) {
        free(buffer);
        return hash_hex;
    }

    FILE *obj_file = fopen(obj_path, "wb");
    if (!obj_file) {
        printf("cannot create object file :(\n");
        free(buffer);
        free(hash_hex);
        return NULL;
    }
    
    fwrite(buffer, sizeof(char), file_size, obj_file);

    fclose(obj_file);
    free(buffer);
    
    return hash_hex; 
}