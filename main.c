#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>

#include "func_sha.h"

#include "exist_dir.h"

#define MAX_INPUT 1024
#define MAX_ARGS 64

// получить абсолютный путь переданного объекта
void get_repo_path(char *name, char **path) {
    *path = realpath(name, NULL);
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
                fprintf(index_temp, "%s %s %s\n", st, file_name, hash);
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

    fclose(index_read);
    fclose(index_temp);

    remove(".mygit/index");
    rename(".mygit/index.tmp", ".mygit/index");
}

// read the file content
// store the blob object in database (.mygit/objects)
char* create_blob(const char* file_path) {
    FILE *f = fopen(file_path, "rb");
    if (!f) {
        printf("Cannot open file %s\n", file_path);
        return NULL;
    }

    // получаем размер файла в байтах
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    // выделяем память для чтения содержимого файла
    unsigned char *buffer = malloc(file_size);
    if (!buffer) {
        printf("mem error :(");
        fclose(f);
        return NULL;
    }

    size_t bytes_read = fread(buffer, sizeof(char), file_size, f);
    fclose(f);

    // if (bytes_read != file_size) {
    //     printf("read error :(\n");
    //     free(buffer);
    //     return NULL;
    // }

    char *hash_hex = malloc(41);
    compute_sha1(buffer, bytes_read, hash_hex);

    char dir_path[256];
    snprintf(dir_path, sizeof(dir_path), ".mygit/objects/%.2s", hash_hex);

    // проверка на существование данной директории
    if (!directory_exists(dir_path)) { 
        mkdir(dir_path, 0755);
    }

    char obj_path[512];
    snprintf(obj_path, sizeof(obj_path), ".mygit/objects/%.2s/%s", hash_hex, hash_hex + 2);
    if (file_exists(obj_path)) {
        free(buffer);
        return hash_hex;
    }

    FILE *obj_file = fopen(obj_path, "wb");
    if (!obj_file) {
        printf("cannot create object file :(\n");
        free(buffer);
        free(hash_hex);
        return NULL;
    }
    
    fwrite(buffer, sizeof(char), file_size, obj_file);

    fclose(obj_file);
    free(buffer);
    
    return hash_hex; 
}

// инициализация репозитория
void init_repository() {
    if (directory_exists(".mygit")) {
        printf("Repository already exist\n");
        return;
    }

    mkdir(".mygit", 0755);
    mkdir(".mygit/objects", 0755);
    mkdir(".mygit/refs", 0755);
    mkdir(".mygit/refs/heads", 0755);

    FILE* head = fopen(".mygit/HEAD", "w");
    fprintf(head, "ref: refs/heads/master\n");
    fclose(head);

    FILE *index = fopen(".mygit/index", "w");
    fclose(index);

    char dirname[] = ".mygit";

    char* path;
    get_repo_path(dirname, &path);
    printf("Initialized empty Git repository in path:");
    if (path == NULL) {
        printf("[cannot find]");
    } else {
        printf("[%s]", path);
        free(path);
    }

    printf("\n");
}


// create BLOB object from the content
// update index to include the file
void add_file(char *file_name) {
    if (!file_exists(file_name)) {
        printf("File %s not found", file_name);
        return;
    }

    char *hash = create_blob(file_name);
    if (!hash) {
        printf("failed to add file :(\n");
    }
    index_update("A", file_name, hash);
    printf("Adding file: %s\n", file_name);


    free(hash);
}

// main func for add command
void add_command(char **args, int n) {
    char* path;
    get_repo_path(".", &path);
    
    for (int i =0; i < n; ++i) {
        char name_arg[MAX_INPUT];
        strcpy(name_arg, path);
        strcat(name_arg, "/");
        strcat(name_arg, args[i]);

        if (!path_exists(name_arg)) {
            printf("Path %s not found\n", name_arg);
            return;
        }

        if (file_exists(name_arg)) {
            add_file(name_arg);
        }
        else if (directory_exists(name_arg)) {
            // add_directory();
        }

    }

    free(path);
}

void parse_argument(char *line, char **func, char **args, int *arg_count) {
    char* token = strtok(line, " ");
    if (token == NULL) return;

    *func = token;

    while ((token = strtok(NULL, " ")) != NULL && *arg_count < MAX_ARGS) {
        args[*arg_count] = token;
        (*arg_count)++;
    }
}

void show_help() {
    printf("\nAvailable commands:\n");
    printf("  init [path]    Initialize a new repository\n");
    printf("  help           Show this help message\n");
    printf("  add            Add file contents to the index\n");
    printf("  exit           Exit the program\n\n");
}

int main() {

    char buff[MAX_INPUT];
    while (1) {
        printf("mygit> ");


        if (fgets(buff, sizeof(buff), stdin) == NULL) {
            break;
        }

        buff[strcspn(buff, "\n")] = '\0';
        if (buff[0] == '\0') continue;

        char* command;
        char* args[MAX_ARGS];
        int count_args = 0;

        char buff_copy[MAX_INPUT];
        strcpy(buff_copy, buff);
        parse_argument(buff_copy, &command, args, &count_args);

        if (command != NULL && strcmp(command, "help") == 0) {
            show_help();
        }
        
        else if (command != NULL && strcmp(command, "exit") == 0) {
            break;
        }

        else if (command != NULL && strcmp(command, "add") == 0) {
            add_command(args, count_args);

        }

        else if (command != NULL && strcmp(command, "init") == 0) {
            init_repository();
            continue;
        }

        else {
            printf("Unknown command\n");

        }
    }

    return 0;
}