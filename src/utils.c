#include "utils.h"
#include <stdbool.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int is_detached_head(void) {
    FILE *head = fopen(".mygit/HEAD", "r");
    if (!head) return 0;
    char line[256];
    if (fgets(line, sizeof(line), head) == NULL) {
        fclose(head);
        return 0;
    }
    fclose(head);
    return (strncmp(line, "ref:", 4) != 0);
}

char* resolve_commit(const char *name) {
    if (!name || name[0] == '\0') return NULL;

    char branch_path[512];
    snprintf(branch_path, sizeof(branch_path), ".mygit/refs/heads/%s", name);
    FILE *f = fopen(branch_path, "r");
    if (f) {
        char *hash = (char*)malloc(41);
        if (fgets(hash, 41, f) == NULL) {
            free(hash);
            fclose(f);
        } else {
            hash[strcspn(hash, "\n")] = '\0';
            fclose(f);
            return hash;
        }
    }

    char obj_path[512];
    build_object_path(name, obj_path);
    if (file_exists(obj_path)) {
        return strdup(name);
    }

    return NULL;
}
