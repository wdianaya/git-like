#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include "status.h"
#include "commit.h"
#include "object.h"
#include "utils.h"
#include "sha1.h"

#define MAX_PATH 1024

// вспомогательная функция для проверки, есть ли файл в индексе
static int file_in_index_local(const char *filename) {
    FILE *index = fopen(".mygit/index", "r");
    if (!index) return 0;
    char line[1024], status[2], file[512], hash[41];
    while (fgets(line, sizeof(line), index)) {

        if (sscanf(line, "%1s %511s %40s", status, file, hash) != 3) continue;
        if (strcmp(file, filename) == 0) { 
            fclose(index); 
            return 1; 
        }
    }
    fclose(index);
    return 0;
}

// xеш без создания blob
char* compute_file_hash(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    SHA_CTX ctx;

    sha1_ctx_init(&ctx);
    unsigned char buffer[65536];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), f)) > 0)
        sha1_ctx_update(&ctx, buffer, bytes);
    fclose(f);
    char *hash = (char*)malloc(41);
    sha1_ctx_final(&ctx, hash);
    return hash;
}

// Вывод staged файлов
void print_index_changes() {
    FILE *index = fopen(".mygit/index","r");
    if (!index) {
        return;
    }
    char line[1024];
    int empty = 1;

    printf("Changes to be committed:\n");
    while (fgets(line,sizeof(line),index)) {
        char status[2], file[512], hash[41];
        if (sscanf(line, "%1s %511s %40s", status, file, hash) != 3) continue;
        empty = 0;

        if (strcmp(status,"A") == 0) {
            printf("    new file: %s\n", file);
        }

        else if (strcmp(status,"M") == 0) {
            printf("    modified: %s\n", file);
        }

        else if (strcmp(status,"D") == 0) {
            printf("    deleted: %s\n", file);
        }
    }
    if (empty) {
        printf("    nothing to commit\n");
    }
    fclose(index);
}

// Поиск modified и deleted
static void check_commit_files() {
    char *head = get_head_commit();
    if (!head) return;
    char path[MAX_PATH];
    build_object_path(head, path);
    FILE *commit = fopen(path, "r");
    free(head);
    if (!commit) return;

    printf("\nChanges not staged for commit:\n");
    char line[1024];
    int any = 0;
    while (fgets(line, sizeof(line), commit)) {
        char status[2], file[512], hash[41];
        if (strncmp(line, "parent:", 7) == 0 || strncmp(line, "date:", 5) == 0 || strncmp(line, "message:", 8)== 0 || line[0] == '\n') continue;
        if (sscanf(line, "%1s %511s %40s", status, file, hash) != 3) continue;
        if (strcmp(status, "D") == 0) continue;
        if (file_in_index_local(file)) continue;

        if (!file_exists(file)) {
            printf("    deleted:  %s\n", file);
            any = 1;
            continue;
        }
        char *cur = compute_file_hash(file);
        if (!cur) continue;
        if (strcmp(cur, hash) != 0) {
            printf("    modified: %s\n", file);
            any = 1;
        }
        free(cur);
    }
    if (!any) printf("    (nothing)\n");
    fclose(commit);
}

// поиск новых файлов
static void find_untracked(const char *dir) {
    DIR *d = opendir(dir);
    if (!d) {
        return;
    }
    struct dirent *entry;
    while ((entry = readdir(d)) != NULL) {
        if (strcmp(entry->d_name,".") == 0 || strcmp(entry->d_name,"..") == 0) {
            continue;
        }

        if (strcmp(entry->d_name,".mygit") == 0) {
            continue;
        }

        if (entry->d_name[0] == '.') {
            continue;
        }
        char path[MAX_PATH];
        if (strlen(dir) == 0) {
            snprintf(path,sizeof(path),"%s",entry->d_name);
        } else {
            snprintf(path,sizeof(path),"%s/%s",dir,entry->d_name);
        }
            


        if (directory_exists(path)) {
            find_untracked(path);
        }

        else if (file_exists(path)) {
            if (!get_last_commit_hash(path) && !file_in_index_local(path)) {
                printf("    untracked: %s\n", path);
            }
        }
    }

    closedir(d);
}

void status_command() {
    if (!directory_exists(".mygit")) {
        fprintf(stderr, "repository not initialized\n");
        return;
    }
    if (is_detached_head())
        printf("[detached HEAD]\n");

    print_index_changes();
    check_commit_files();
    printf("\nUntracked files:\n");
    find_untracked("");
    printf("\n");
}