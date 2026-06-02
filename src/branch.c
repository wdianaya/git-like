#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include "branch.h"
#include "commit.h"
#include "utils.h"

#define BRANCHES_DIR ".mygit/refs/heads"

// получить имя текущей ветки из HEAD
char* get_current_branch_name() {
    FILE *head = fopen(".mygit/HEAD", "r");
    if (!head) {
        return NULL;
    }

    char ref_line[256];
    if (fgets(ref_line, sizeof(ref_line), head) == NULL) {
        fclose(head);
        return NULL;
    }
    fclose(head);

    ref_line[strcspn(ref_line, "\n")] = '\0';

    char *prefix = "ref: refs/heads/";
    if (strncmp(ref_line, prefix, strlen(prefix)) != 0) {
        return NULL; // detached Head
    }

    char *branch_name = (char*)malloc(256 * sizeof(char));
    strncpy(branch_name, ref_line + strlen(prefix), 256);
    branch_name[255] = '\0';
    return branch_name;
}

// получить хеш коммита для указанной ветки
char* get_branch_commit(const char *branch_name) {
    char branch_path[512];
    snprintf(branch_path, sizeof(branch_path), ".mygit/refs/heads/%s", branch_name);

    FILE *f = fopen(branch_path, "r");
    if (!f) {
        return NULL;
    }

    char *hash = (char*)malloc(41 * sizeof(char));
    if (fgets(hash, 41, f) == NULL) {
        free(hash);
        fclose(f);
        return NULL;
    }

    hash[strcspn(hash, "\n")] = '\0';
    fclose(f);
    return hash;
}

// обновить ветку на указанный коммит
void update_branch_head(const char *branch_name, const char *commit_hash) {
    char branch_path[512];
    snprintf(branch_path, sizeof(branch_path), "%s/%s", BRANCHES_DIR, branch_name);
    FILE *f = fopen(branch_path, "w");
    if (!f) {
        fprintf(stderr, "cannot update branch %s\n", branch_name);
        return;
    }

    fprintf(f, "%s\n", commit_hash);
    fclose(f);
}

// вывести список всех веток
void list_branches() {
    DIR *dir = opendir(BRANCHES_DIR);
    if (!dir) {
        fprintf(stderr, "cannot open branches directory\n");
        return;
    }

    char *current_branch = get_current_branch_name();

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') {
            continue;
        }

        if (current_branch && strcmp(entry->d_name, current_branch) == 0) {
            printf("* %s\n", entry->d_name);
        } else {
            printf("  %s\n", entry->d_name);
        }
    }

    closedir(dir);
    free(current_branch);
}

// создать новую ветку от текущего коммита
void create_branch(const char *branch_name) {
    // проверяем, не существует ли уже такая ветка
    char branch_path[512];
    snprintf(branch_path, sizeof(branch_path), ".mygit/refs/heads/%s", branch_name);

    if (path_exists(branch_path)) {
        fprintf(stderr, "branch '%s' already exists\n", branch_name);
        return;
    }

    // получаем текущий коммит
    char *current_commit = get_head_commit();
    if (!current_commit) {
        fprintf(stderr, "no commits yet\n");
        return;
    }

    // создаём файл ветки с текущим коммитом
    update_branch_head(branch_name, current_commit);
    free(current_commit);

    printf("Created branch '%s'\n", branch_name);
}

// основная команда branch
void branch_command(char **args, int count) {
    if (!directory_exists(".mygit")) {
        fprintf(stderr, "repository not initialized\n");
        return;
    }

    if (count == 0) {
        // без аргументов - вывести список
        list_branches();
    } else if (count == 1) {
        // с аргументом - создать ветку
        create_branch(args[0]);
    } else {
        fprintf(stderr, "pleusage: branch [name]\n");
    }
}