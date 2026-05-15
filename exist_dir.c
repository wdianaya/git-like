#include "exist_dir.h"
#include <stdbool.h>
#include <sys/stat.h>

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