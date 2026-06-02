#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "checkout.h"
#include "commit.h"
#include "utils.h"
#include "diff.h"
#include "status.h"

#ifdef _WIN32
    #include <direct.h>
    #include <io.h>
    #define MKDIR(path) _mkdir(path)
#else
    #include <sys/stat.h>
    #include <unistd.h>
    #define MKDIR(path) mkdir(path, 0755)
#endif

int find_file_in_commit(const char *commit_hash, const char *filename, char *blob_hash) {
    char path[512];
    build_object_path(commit_hash, path);
    FILE *f = fopen(path, "r");
    if (!f) return 0;

    char line[1024];
    while (fgets(line, sizeof(line), f)) {
        char status[2], file[512], hash[41];
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

    build_object_path(blob_hash, obj_path);
    FILE *obj = fopen(obj_path, "rb");
    if (!obj) {
        return 0;
    }

    // проверка что директория родитель существует
    char dir[512];
    strncpy(dir, filename, sizeof(dir) - 1);
    char *slash = strrchr(dir, '/');
    if (slash) {
        *slash = '\0';
        // рекурсивно создаем директории
        if (!directory_exists(dir)) MKDIR(dir);
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

static int working_tree_clean() {
    char *head = get_head_commit();
    if (!head) return 1; // выходим при отсутсвиии коммита

    char path[512];
    build_object_path(head, path);
    free(head);

    FILE *f = fopen(path, "r");
    if (!f) return 1;

    char line[1024];
    int clean = 1;

    while (fgets(line, sizeof(line), f)) {
        char status[2], file[512], hash[41];
        if (strncmp(line, "parent:", 7)  == 0 || strncmp(line, "date:", 5) == 0 || strncmp(line, "message:", 8) == 0 || line[0] == '\n') continue;
        if (sscanf(line, "%1s %511s %40s", status, file, hash) != 3) continue;
        if (strcmp(status, "D") == 0) continue;

        if (!file_exists(file)) {
            fprintf(stderr, "  modified (deleted): %s\n", file);
            clean = 0;
            continue;
        }
        
        // compare hash
        extern char* compute_file_hash(const char *path);
        char *cur = compute_file_hash(file);
        if (!cur) continue;
        if (strcmp(cur, hash) != 0) {
            fprintf(stderr, "  modified: %s\n", file);
            clean = 0;
        }
        free(cur);
    }
    fclose(f);
    return clean;
}

// удаляем все файлы затреканные в указанном коммите (очищаем рабочее дерево)
static void delete_commit_files(const char *commit_hash) {
    char path[512];
    build_object_path(commit_hash, path);
    FILE *f = fopen(path, "r");
    if (!f) return;

    char line[1024];
    while (fgets(line, sizeof(line), f)) {
        char status[2], file[512], hash[41];
        if (strncmp(line, "parent:", 7)  == 0 ||strncmp(line, "date:", 5)== 0 ||strncmp(line, "message:", 8) == 0 ||line[0] == '\n') continue;
        if (sscanf(line, "%1s %511s %40s", status, file, hash) != 3) continue;
        if (strcmp(status, "D") == 0) continue;
        if (file_exists(file)) remove(file);
    }
    fclose(f);
}

// восстанавливаем все файлы из коммита, пропуская удаленные
static void restore_commit_files(const char *commit_hash) {
    char path[512];
    build_object_path(commit_hash, path);
    FILE *f = fopen(path, "r");
    if (!f) return;

    char line[1024];
    while (fgets(line, sizeof(line), f)) {
        char status[2], file[512], hash[41];
        if (strncmp(line, "parent:", 7)  == 0 ||strncmp(line, "date:", 5)== 0 ||strncmp(line, "message:", 8) == 0 ||line[0] == '\n') continue;
        
        if (sscanf(line, "%1s %511s %40s", status, file, hash) != 3) continue;
        if (strcmp(status, "D") == 0) continue;
        if (strcmp(hash, "0000000000000000000000000000000000000000") == 0) continue;
        restore_blob(hash, file);
    }
    fclose(f);
}

// добавляем в head то на что указывает в текущий момент
static void set_head(const char *commit_hash, const char *branch_name) {
    FILE *head = fopen(".mygit/HEAD", "w");
    if (!head) return;
    if (branch_name) {

        fprintf(head, "ref: refs/heads/%s\n", branch_name);
    } else {
        fprintf(head, "%s\n", commit_hash);
    }
    fclose(head);
}

static char* branch_name_for(const char *name) {
    char branch_path[512];
    snprintf(branch_path, sizeof(branch_path), ".mygit/refs/heads/%s", name);
    if (file_exists(branch_path)) return strdup(name);
    return NULL;
}

// восстанавливаем репо для текущего коммита
static void checkout_repo(const char *name) {
    char *branch = branch_name_for(name);
    char *target_hash = resolve_commit(name);

    if (!target_hash) {
        fprintf(stderr, "commit or branch '%s' not found\n", name);
        free(branch);
        return;
    }

    // проверка не пустая ли директория
    if (!working_tree_clean()) {
        fprintf(stderr, "checkout aborted: working tree has modifications\n");
        free(branch);
        free(target_hash);
        return;
    }

    char *current = get_head_commit();

    if (current) {
        delete_commit_files(current);
        free(current);
    }

    // восстанавливаем нужные файлы из коммита
    restore_commit_files(target_hash);

    // обновление хеад
    set_head(target_hash, branch);
    
    // очищаем индекс при переключении
    FILE *idx = fopen(".mygit/index", "w");
    if (idx) fclose(idx);

    if (branch) {
        printf("Switched to branch '%s'\n", branch);
    } else {
        printf("HEAD is now at %.8s (detached)\n", target_hash);
        printf("You are in 'detached HEAD' state.\n");
        printf("Only 'checkout' and 'branch' commands are available.\n");
    }

    free(branch);
    free(target_hash);
}

void checkout_command(char **args, int count) {
    if (!directory_exists(".mygit")) {

        fprintf(stderr, "repository not initialized\n");
        return;
    }

    if (count == 0) {
        fprintf(stderr, "usage: checkout <commit|branch> [file]\n");
        return;
    }
    if (count == 1) {
        checkout_repo(args[0]);
        return;
    }

    if (is_detached_head()) {
        fprintf(stderr, "in detached HEAD: only 'checkout <commit|branch>' (no file) is allowed\n");
        return;
    }
    char *commit_hash = resolve_commit(args[0]);
    if (!commit_hash) {
        fprintf(stderr, "commit or branch '%s' not found\n", args[0]);
        return;
    }

    const char *filename = args[1];
    char blob_hash[41];

    if (!find_file_in_commit(commit_hash, filename, blob_hash)) {
        printf("file '%s' not found in commit %s\n", filename, commit_hash);
        free(commit_hash);
        return;
    }
    if (strcmp(blob_hash, "0000000000000000000000000000000000000000") == 0) {
        
        printf("file was deleted  in commit %s\n", filename);
        free(commit_hash);
        return;
    }
    if (!restore_blob(blob_hash, filename)) {
        printf("cannot restore file '%s'\n", filename);
        free(commit_hash);
        return;
    }

    printf("Restored '%s' from %.8s\n", filename, commit_hash);
    
    free(commit_hash);
}