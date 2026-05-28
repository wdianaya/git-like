#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "utils.h"
#include "object.h"

#define MAX_INPUT 1024
#define MAX_ARGS 64

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

// create BLOB object from the content
// update index to include the file
void add_file(char *real_path, char *repo_path) {
    if (!file_exists(real_path)) {
        fprintf(stderr, "File %s not found\n", real_path);
        return;
    }

    char *hash = create_blob(real_path);

    if (!hash) {
        fprintf(stderr, "failed to add file :(\n");
        return;
    }

    index_update("A", repo_path, hash);
    printf("Adding file: %s\n", repo_path);

    free(hash);
}

// main func for add command
void add_command(char **args, int n) {
    if (!directory_exists(".mygit")) {
        fprintf(stderr, "repo was not found\n");
        return;
    }

    char* path;
    get_repo_path(".", &path);
    
    for (int i =0; i < n; ++i) {
        char name_arg[MAX_INPUT];
        snprintf(name_arg, sizeof(name_arg), "%s", path);
        // strcpy(name_arg, path);
        strcat(name_arg, "/");
        strcat(name_arg, args[i]);

        if (!path_exists(name_arg)) {
            fprintf(stderr, "Path %s not found\n", name_arg);
            return;
        }

        if (file_exists(name_arg)) {
            add_file(name_arg, args[i]);
        }
        else if (directory_exists(name_arg)) {
            // add_directory();
        }
    }

    free(path);
}

// пометить файл удаленным
void remove_file(char *repo_path) {
    index_update(
        "D",
        repo_path,
        "0000000000000000000000000000000000000000"
    );

    printf("Removing file: %s\n", repo_path);
}

// main func for remove
void remove_command(char **args, int count) {
    if (!directory_exists(".mygit")) {
        fprintf(stderr, "repo was not found\n");
        return;
    }

    char* path;
    get_repo_path(".", &path);
    
    for (int i =0; i < count; ++i) {
        char name_arg[MAX_INPUT];
        snprintf(name_arg, sizeof(name_arg), "%s", path);
        // strcpy(name_arg, path);
        strcat(name_arg, "/");
        strcat(name_arg, args[i]);

        if (!path_exists(name_arg)) {
            fprintf(stderr, "Path %s not found\n", name_arg);
            return;
        }

        if (file_exists(name_arg)) {
            remove_file(args[i]);
        }
        else if (directory_exists(name_arg)) {
            
        }
    }

    free(path);
}