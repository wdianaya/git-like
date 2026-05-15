#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>

#include <openssl/sha.h> 

#include "exist_dir.h"

#define MAX_INPUT 1024
#define MAX_ARGS 64

void hash_func() {
    const unsigned char str[] = "Original string";
    unsigned char hash[20];
    SHA1(str, sizeof(str) - 1, hash);
    printf("%s\n", hash);
    printf("%s\n", str);
}

// получить абсолютный путь переданного объекта
void get_repo_path(char *name, char **path) {
    *path = realpath(name, NULL);
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

// read the file content
// create BLOB object from the content
// store the blob object in database (.mygit/objects)
// update index to include the file
void add_file(char *path) {
    if (!path_exists(path)) {
        printf("Path %s not found", path);
        return;
    }

    FILE* content = fopen(path, "rb");


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
            printf("Path %s not found", name_arg);
            return;
        }

        if (file_exists(name_arg)) {
            // add_file(name_arg);
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
            // for (int i =0; i < count_args; ++i) {
            //     printf("%s ", args[i]);
            // }
            // printf("\n");

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