#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "diff.h"
#include "commit.h"
#include "utils.h"

#define MAX_FILES 1024

int find_file(FileEntry files[], int count, const char *filename) {
    for (int i = 0; i < count; ++i) {
        if (strcmp(files[i].filename, filename) == 0) {
            return i;
        }
    }
    return -1;
}


int load_commit_files(const char *hash, FileEntry files[], int max_files) {
    char path[512];
    build_object_path(hash, path);

    FILE *f = fopen(path, "r");

    if (!f) return 0;

    char line[1024];
    int count = 0;

    while (fgets(line, sizeof(line), f)) {
        // пропускаем метаданные
        if (strncmp(line, "parent:", 7) == 0 || strncmp(line, "date:", 5) == 0 
            || strncmp(line, "message:", 8) == 0 || strcmp(line, "\n") == 0) {
            continue;
        }

        if (count >= max_files) {
            break;
        }

        sscanf(line,
               "%1s %511s %40s",
               files[count].status,
               files[count].filename,
               files[count].hash);
        count++;
    }

    fclose(f);
    return count;
}

void diff_command(char **args, int count) {
    if (count == 0) {
        printf("commit hash required\n");
        return;
    }

    char *target_hash = args[0];
    char *current_hash = get_head_commit();

    if (!current_hash) {
        printf("No commits\n");
        return;
    }

    FileEntry current_files[MAX_FILES];
    FileEntry target_files[MAX_FILES];

    int current_count = load_commit_files(current_hash,current_files,MAX_FILES);
    int target_count = load_commit_files(target_hash, target_files, MAX_FILES);

    printf("\nDiff:\n\n");

    // cmp current -> target
    for (int i = 0; i < current_count; ++i) {
        int idx = find_file(target_files,target_count,current_files[i].filename);
        // added
        if (idx == -1) {
            printf("ADDED: %s [%s]\n",current_files[i].filename,current_files[i].hash);
            continue;
        }
        // changed
        if (strcmp(current_files[i].hash, target_files[idx].hash) != 0) {
            printf("MODIFIED: %s\n", current_files[i].filename);

            printf("  current: %s\n",current_files[i].hash);
            printf("  target : %s\n",target_files[idx].hash);
        }
    }
    // del
    for (int i = 0; i < target_count; ++i) {
        int idx = find_file(current_files,current_count,target_files[i].filename);
        if (idx == -1) {
            printf("DELETED: %s [%s]\n",target_files[i].filename,target_files[i].hash);
        }
    }

    free(current_hash);
}