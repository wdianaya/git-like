#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#include "commit.h"
#include "utils.h"
#include "object.h"
#include "sha1.h"
#include "branch.h"

#ifdef _WIN32
    #include <direct.h>
    #define MKDIR(path) _mkdir(path)
#else
    #include <sys/stat.h>
    #define MKDIR(path) mkdir(path, 0755)
#endif

#define MAX_COMMIT_SIZE 8192
#define MAX_INP 1024

// получить hash текущего commit 
char* get_head_commit() {
    FILE *head = fopen(".mygit/HEAD", "r");
    if (!head) {
        return NULL;
    }
    char ref_line[256];
    fgets(ref_line, sizeof(ref_line), head);
    fclose(head);

    ref_line[strcspn(ref_line, "\n")] = '\0';
    char ref_path[256];

    sscanf(ref_line, "ref: %s", ref_path);

    char full_ref[512];
    sprintf(full_ref, ".mygit/%s", ref_path);

    FILE *ref_file = fopen(full_ref, "r");

    if (!ref_file) {
        return NULL;
    }

    char* hash = (char*)malloc(41);

    if (fgets(hash, 41, ref_file) == NULL) {
        free(hash);
        fclose(ref_file);
        return NULL;
    }

    hash[strcspn(hash, "\n")] = '\0';

    fclose(ref_file);
    return hash;
}

// сохранить commit object
void save_commit_object(const char *hash, const char *content) {
    char dir_path[256];
    sprintf(dir_path, ".mygit/objects/%.2s", hash);

    if (!directory_exists(dir_path)) {
        MKDIR(dir_path);
    }

    char obj_path[512];

    sprintf(obj_path,
            ".mygit/objects/%.2s/%s",
            hash,
            hash + 2);

    FILE *obj = fopen(obj_path, "w");

    if (!obj) {
        fprintf(stderr, "cannot create commit object\n");
        return;
    }

    fprintf(obj, "%s", content);

    fclose(obj);
}

// обновить текущую ветку (ту, на которую указывает HEAD)
void update_current_branch(const char *hash) {
    char *branch_name = get_current_branch_name();
    if (branch_name) {
        update_branch_head(branch_name, hash);
        free(branch_name);
    }
}

// очистить index
void clear_index() {
    FILE *index = fopen(".mygit/index", "w");
    if (index) {
        fclose(index);
    }
}

void commit_command(char **args, int count) {
    if (!directory_exists(".mygit")) {
        fprintf(stderr, "repository not initialized\n");
        return;
    }

    char message[MAX_INP] = "";
    for (int i =0; i < count;++i) {
        strncat(message, args[i], MAX_INP - strlen(message) - 1);
        if (i != count - 1) {
            strncat(message, " ", MAX_INP - strlen(message) - 1);
        }
    }

    FILE *index = fopen(".mygit/index", "r");

    if (!index) {
        fprintf(stderr, "cannot open index\n");
        return;
    }

    // читаем index
    char index_content[4096] = "";
    char line[1024];

    int is_empty = 1;

    while (fgets(line, sizeof(line), index)) {
        strcat(index_content, line);
        is_empty = 0;
    }

    fclose(index);

    if (is_empty) {
        fprintf(stderr, "nothing to commit\n");
        return;
    }

    // parent hash
    char *parent_hash = get_head_commit();

    // время
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);

    char date[64];

    strftime(date, sizeof(date),
             "%Y-%m-%d %H:%M:%S",
             tm_info);

    // формируем commit content
    char commit_content[MAX_COMMIT_SIZE];

    snprintf(commit_content,
             sizeof(commit_content),
             "parent: %s\n"
             "date: %s\n"
             "message: %s\n"
             "\n"
             "%s",
             parent_hash ? parent_hash : "NULL",
             date,
             message,
             index_content);

    // hash commit
    char commit_hash[41];

    compute_sha1((unsigned char*)commit_content, strlen(commit_content), commit_hash);

    // сохраняем object
    save_commit_object(commit_hash, commit_content);

    // обновляем branch
    update_current_branch(commit_hash);

    // очищаем index
    clear_index();

    printf("Committed as %s\n", commit_hash);

    if (parent_hash) {
        free(parent_hash);
    }
}

void create_initial_commit() {
    time_t now = time(NULL);

    struct tm *tm_info = localtime(&now);

    char date[64];

    strftime(date,
             sizeof(date),
             "%Y-%m-%d %H:%M:%S",
             tm_info);

    char commit_content[1024];

    snprintf(commit_content,
             sizeof(commit_content),
             "parent: NULL\n"
             "date: %s\n"
             "message: initial commit\n",
             date);

    char commit_hash[41];

    compute_sha1(
        (unsigned char*)commit_content,
        strlen(commit_content),
        commit_hash
    );

    save_commit_object(commit_hash, commit_content);
    update_current_branch(commit_hash);
    // update_branch_head(commit_hash);

    printf("Initial commit: %s\n", commit_hash);
}