#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <dirent.h>           

#include "utils.h"
#include "object.h"
#include "commit.h"

#define MAX_INPUT 1024
#define MAX_ARGS 64

int file_in_index(const char *filename) {
    FILE *index = fopen(".mygit/index", "r");
    if (!index) {
        return 0;
    }

    char line[1024];
    char st[2];
    char file[512];
    char hash[41];

    while (fgets(line, sizeof(line), index)) {
        if (sscanf(line, "%1s %511s %40s", st, file, hash) == 3) {
            if (strcmp(file, filename) == 0) {
                fclose(index);
                return 1;
            }
        }
    }

    fclose(index);
    return 0;
}

// обновляет index file
void index_update(const char *status, const char *file_name, const char *hash) {
    FILE *index_read = fopen(".mygit/index", "r");

    // создаем временный файл 
    FILE *index_temp = fopen(".mygit/index.tmp", "w");
    
    char line[1024];
    bool updated = false;

    if (index_read) {
        while (fgets(line, sizeof(line), index_read)) {
            line[strcspn(line, "\n")] = '\0';
            char st[2];
            char cur_file[512];
            char cur_hash[41];

            sscanf(line, "%s %s %s", st, cur_file, cur_hash);

            if (strcmp(cur_file, file_name) == 0) {
                fprintf(index_temp, "%s %s %s\n", status, file_name, hash);
                updated = true;
            } else {
                fprintf(index_temp, "%s\n", line);

            }

        }
    }
    // если файла не было добавляем запись о нем
    if (!updated) {
        fprintf(index_temp, "%s %s %s\n", status, file_name, hash);
    }

    if (index_read) {
        fclose(index_read);
    }
    fclose(index_temp);

    remove(".mygit/index");
    rename(".mygit/index.tmp", ".mygit/index");
}

char *get_file_hash_from_head(const char *filename) {
    char *commit_hash = get_head_commit();
    if (!commit_hash) {
        return NULL;
    }
    char commit_path[512];
    build_object_path(commit_hash, commit_path);
    FILE *f = fopen(commit_path, "r");
    free(commit_hash);

    if (!f) {
        return NULL;
    }

    char line[1024];
    while (fgets(line, sizeof(line), f)) {
        char status[2];
        char file[512];
        char hash[41];
        if (sscanf(line, "%1s %511s %40s", status, file, hash) != 3) continue;
        if (strcmp(file, filename) == 0) {
            fclose(f);
            char *result = (char*)malloc(41);
            strcpy(result, hash);
            return result;
        }
    }

    fclose(f);
    return NULL;
}

void stage_file(const char *real_path, const char *repo_path) {
    char *old_hash = get_file_hash_from_head(repo_path);
    char *new_hash = create_blob(real_path);

    if (!new_hash) {
        free(old_hash);
        return;
    }

    if (!old_hash) {
        index_update("A", repo_path, new_hash);
        printf("added: %s\n", repo_path);
    }
    else {
        if (strcmp(old_hash, new_hash) != 0) {
            index_update("M", repo_path, new_hash);
            printf("modified: %s\n", repo_path);
        }
    }
    free(old_hash);
    free(new_hash);
}

void add_directory(const char *real_dir, const char *repo_dir) {
    DIR *dir = opendir(real_dir);
    if (!dir) {
        return;
    }
    struct dirent *entry;
    while ((entry = readdir(dir))) {
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        if (entry->d_name[0] == '.') {
            continue;
        }
        if (strcmp(entry->d_name, ".mygit") == 0) {
            continue;
        }
        char full_path[1024];
        char repo_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", real_dir, entry->d_name);
        snprintf(repo_path, sizeof(repo_path), "%s/%s", repo_dir, entry->d_name);

        if (file_exists(full_path)) {
            stage_file(full_path, repo_path);
        }
        else if (directory_exists(full_path)) {
            add_directory(full_path, repo_path);
        }
    }
    closedir(dir);
}

void detect_deleted_files(const char *prefix) {
    char *commit_hash = get_head_commit();
    if (!commit_hash) {
        return;
    }
    char commit_path[512];
    build_object_path(commit_hash, commit_path);
    free(commit_hash);
    FILE *f = fopen(commit_path, "r");
    if (!f) {
        return;
    }

    char line[1024];

    while (fgets(line, sizeof(line), f)) {  
        if (strncmp(line, "parent:", 7) == 0 || strncmp(line, "date:", 5) == 0 || strncmp(line, "message:", 8) == 0 || line[0] == '\n') {
            continue;
        }
        char status[2];
        char file[512];
        char hash[41];
        if (sscanf(line, "%1s %511s %40s", status, file, hash) != 3) {
            continue;
        }

        if (strlen(prefix) > 0 && strcmp(prefix, ".") != 0) {
            if (strncmp(file, prefix, strlen(prefix)) != 0) continue;
        }

        if (!file_exists(file)) {
            index_update("D", file, "0000000000000000000000000000000000000000");
            printf("removed: %s\n", file);
        }
    }
    fclose(f);
}

void add_command(char **args, int count) {
    if (!directory_exists(".mygit")) {
        fprintf(stderr, "repo not found\n");
        return;
    }

    if (is_detached_head()) {
        fprintf(stderr, "cannot add: HEAD is detached\n");
        return;
    }
    
    for (int i = 0;i < count; i++) {
        if (!path_exists(args[i])) {
            fprintf(stderr, "path %s not found\n",args[i]);
            continue;
        }

        if (file_exists(args[i])) {
            stage_file(args[i], args[i]);
        }

        else if (directory_exists(args[i])) {
            add_directory(args[i], args[i]);
            detect_deleted_files(args[i]);
        }
    }
}