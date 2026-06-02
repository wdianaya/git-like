#include "utils.h" 
#include <stdbool.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h> 

bool directory_exists(const char *path) {
    struct stat st={0};
    if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)) {
        return true;
    }
    return false;
}

bool file_exists(const char *path) {
    struct stat st={0};
    if (stat(path, &st) == 0 && S_ISREG(st.st_mode)) {
        return true;
    }
    return false;
}

bool path_exists(const char *path) {
    struct stat st={0};
    if (stat(path, &st) == 0) {
        return true;
    }
    return false;
}

void get_repo_path(char *name, char **path) {
    *path = realpath(name, NULL);
}

void build_object_path(const char *hash, char *path) {
    sprintf(path, ".mygit/objects/%.2s/%s", hash, hash + 2);
}

void play() {
    printf("12\n");
    return;
}