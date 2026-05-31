#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "checkout.h"

int find_file_in_commit(const char *commit_hash, const char *filename, char *blob_hash) {
    char path[512];

    sprintf(path, ".mygit/objects/%.2s/%s", commit_hash, commit_hash + 2);

    FILE *f = fopen(path, "r");

    if (!f) {
        return 0;
    }

    char line[1024];

    while (fgets(line, sizeof(line), f)) {
        char status[2];
        char file[512];
        char hash[41];

        // пытаемся прочитать file entry
        if (sscanf(line, "%1s %511s %40s", status, file, hash) == 3) {
            if (strcmp(file, filename) == 0) {
                strcpy(blob_hash, hash);
                fclose(f);
                return 1;
            }
        }
    }

    fclose(f);
    return 0;
}

int restore_blob(const char *blob_hash, const char *filename) {
    char obj_path[512];

    sprintf(obj_path, ".mygit/objects/%.2s/%s", blob_hash, blob_hash + 2);

    FILE *obj = fopen(obj_path, "rb");

    if (!obj) {
        return 0;
    }

    FILE *out = fopen(filename, "wb");

    if (!out) {
        fclose(obj);
        return 0;
    }

    char buffer[4096];
    size_t bytes;

    while ((bytes = fread(buffer, 1, sizeof(buffer), obj)) > 0) {
        fwrite(buffer, 1, bytes, out);
    }

    fclose(obj);
    fclose(out);

    return 1;
}

void checkout_command(char **args, int count) {
    if (count < 2) {
        printf("usage: checkout <commit> <file>\n");
        return;
    }

    char *commit_hash = args[0];
    char *filename = args[1];

    char blob_hash[41];

    if (!find_file_in_commit(commit_hash, filename, blob_hash)) {
        printf("file not found in commit\n");
        return;
    }
    // deleted file
    if (strcmp(blob_hash, "0000000000000000000000000000000000000000") == 0) {
        printf("file deleted in this commit\n" );
        return;
    }

    if (!restore_blob(blob_hash, filename)) {
        printf("cannot restore file\n");
        return;
    }

    printf("Restored %s from %s\n", filename, commit_hash);
}