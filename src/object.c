#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <openssl/sha.h>

#include "utils.h"
#include "sha1.h"

#ifdef _WIN32
    #include <direct.h>
    #define MKDIR(path) _mkdir(path)
#else
    #include <sys/stat.h>
    #define MKDIR(path) mkdir(path, 0755)
#endif

#define HASH_SIZE 41
#define BUFFER_SIZE 65526 // 64кб

// read the file content
// store the blob object in database (.mygit/objects)
char* create_blob(const char* file_path) {
    FILE *f = fopen(file_path, "rb");
    if (!f) {
        fprintf(stderr, "Cannot open file %s\n", file_path);
        return NULL;
    }

    // получаем размер файла в байтах
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    unsigned char *buffer = (unsigned char*)malloc(sizeof(unsigned char) * BUFFER_SIZE);
    if (!buffer) {
        fclose(f);
        return NULL;
    }
    // инициализация потокового хеширования
    SHA_CTX ctx;
    sha1_ctx_init(&ctx);

    size_t total_read = 0;
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, f)) > 0) {
        sha1_ctx_update(&ctx, buffer, bytes_read);
        total_read += bytes_read;
    }
    fclose(f);
    
    if (total_read != (size_t)file_size) {
        fprintf(stderr, "read error :(\n");
        free(buffer);
        return NULL;
    }

    // Получаем итоговый хеш
    char *hash_hex = (char*)malloc(HASH_SIZE * sizeof(char));
    if (!hash_hex) {
        free(buffer);
        fprintf(stderr, "mem error :(\n");
        return NULL;
    }
    sha1_ctx_final(&ctx, hash_hex);

    char dir_path[256];
    snprintf(dir_path, sizeof(dir_path), ".mygit/objects/%.2s", hash_hex);

    // проверка на существование данной директории
    if (!directory_exists(dir_path)) { 
        MKDIR(dir_path);
    }

    char obj_path[512];
    build_object_path(hash_hex, obj_path);
    if (file_exists(obj_path)) {
        free(buffer);
        return hash_hex;
    }

    FILE *obj_file = fopen(obj_path, "wb");
    if (!obj_file) {
        fprintf(stderr, "cannot create object file :(\n");
        free(buffer);
        free(hash_hex);
        return NULL;
    }
    // перечитываем файл для записи в объект
    f = fopen(file_path, "rb");
    if (!f) {
        fprintf(stderr, "Cannot reopen file %s\n", file_path);
        fclose(obj_file);
        free(hash_hex);
        free(buffer);
        return NULL;
    }
    
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, f)) > 0) {
        fwrite(buffer, 1, bytes_read, obj_file);
    }

    fclose(f);
    fclose(obj_file);
    free(buffer);
    
    return hash_hex; 
}