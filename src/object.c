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

    // выделяем память для чтения содержимого файла
    unsigned char *buffer = (char*)malloc(file_size);
    if (!buffer) {
        fprintf(stderr, "mem error :(");
        fclose(f);
        return NULL;
    }

    size_t bytes_read = fread(buffer, sizeof(char), file_size, f);
    fclose(f);

    if (bytes_read != file_size) {
        fprintf(stderr, "read error :(\n");
        free(buffer);
        return NULL;
    }

    char *hash_hex = malloc(41);
    compute_sha1(buffer, bytes_read, hash_hex);

    char dir_path[256];
    snprintf(dir_path, sizeof(dir_path), ".mygit/objects/%.2s", hash_hex);

    // проверка на существование данной директории
    if (!directory_exists(dir_path)) { 
        MKDIR(dir_path);
    }

    char obj_path[512];
    snprintf(obj_path, sizeof(obj_path), ".mygit/objects/%.2s/%s", hash_hex, hash_hex + 2);
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
    
    fwrite(buffer, sizeof(char), file_size, obj_file);

    fclose(obj_file);
    free(buffer);
    
    return hash_hex; 
}