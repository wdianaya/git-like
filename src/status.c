#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>

#include "status.h"
#include "utils.h"
#include "commit.h"
#include "object.h"

#ifdef _WIN32
    #define PATH_SEPARATOR '\\'
#else
    #define PATH_SEPARATOR '/'
#endif

// вспомогательная функция для проверки, есть ли файл в индексе
static int file_in_index(const char *filename) {
    FILE *index = fopen(".mygit/index", "r");
    if (!index) return 0;
    
    char line[1024];
    int found = 0;
    while (fgets(line, sizeof(line), index)) {
        char status[3];
        char file[512];
        char hash[41];
        if (sscanf(line, "%2s %511s %40s", status, file, hash) == 3) {
            if (strcmp(file, filename) == 0) {
                found = 1;
                break;
            }
        }
    }
    fclose(index);
    return found;
}

// получить текущую рабочую директорию 
char* get_current_working_dir() {
    char *path = NULL;
    get_repo_path(".", &path);
    return path;
}

// рекурсивный обход для status
void check_directory_for_status(const char *dir_path, const char *repo_prefix) {
    DIR *dir = opendir(dir_path);
    if (!dir) return;
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // пропускаем . и ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        // пропускаем скрытые файлы
        if (entry->d_name[0] == '.') {
            continue;
        }
        
        char full_path[1024];
        char repo_path[1024];
        
        // строим полный путь 
        snprintf(full_path, sizeof(full_path), "%s%c%s", dir_path, PATH_SEPARATOR, entry->d_name);
        
        // строим путь относительно репозитория
        if (strcmp(repo_prefix, ".") == 0) {
            snprintf(repo_path, sizeof(repo_path), "%s", entry->d_name);
        } else {
            snprintf(repo_path, sizeof(repo_path), "%s%c%s", repo_prefix, PATH_SEPARATOR, entry->d_name);
        }
        
        if (file_exists(full_path)) {
            // получаем статус файла
            char *last_hash = get_last_commit_hash(repo_path);
            char *current_hash = create_blob(full_path);
            
            int in_index = file_in_index(repo_path);
            
            if (!last_hash && current_hash && !in_index) {
                printf("  untracked: %s\n", repo_path);
            } else if (last_hash && current_hash && strcmp(last_hash, current_hash) != 0 && !in_index) {
                printf("  modified: %s (not staged)\n", repo_path);
            }
            
            if (last_hash) free(last_hash);
            if (current_hash) free(current_hash);
            
        } else if (directory_exists(full_path)) {
            // пропускаем .mygit директорию
            if (strcmp(entry->d_name, ".mygit") != 0) {
                check_directory_for_status(full_path, repo_path);
            }
        }
    }
    closedir(dir);
}

void status_command() {
    if (!directory_exists(".mygit")) {
        fprintf(stderr, "repository not initialized\n");
        return;
    }
    
    printf("\nChanges to be committed:\n");
    
    FILE *index = fopen(".mygit/index", "r");
    if (!index) {
        fprintf(stderr, "cannot open index\n");
        return;
    }
    
    char line[1024];
    int empty = 1;
    
    while (fgets(line, sizeof(line), index)) {
        char status[3];
        char filename[512];
        char hash[41];
        
        if (sscanf(line, "%2s %511s %40s", status, filename, hash) == 3) {
            empty = 0;
            
            if (strcmp(status, "A") == 0) {
                printf("  new file: %s\n", filename);
            } else if (strcmp(status, "D") == 0) {
                printf("  deleted: %s\n", filename);
            } else if (strcmp(status, "M") == 0) {
                printf("  modified: %s\n", filename);
            }
        }
    }
    
    fclose(index);
    
    if (empty) {
        printf("  (nothing to commit)\n");
    }
    
    printf("\nChanges not staged for commit:\n");
    
    // используем get_repo_path вместо getcwd
    char *cwd = get_current_working_dir();
    if (cwd) {
        check_directory_for_status(cwd, ".");
        free(cwd);
    } else {
        printf("  (cannot determine working directory)\n");
    }
    
    printf("\n");
}