#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "repository.h"
#include "utils.h"
#include "object.h"
#include "index.h"

#define MAX_INPUT 1024
#define MAX_ARGS 64

void parse_argument(char *line, char **func, char **args, int *arg_count);
void show_help();


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
        
        else if (command != NULL && strcmp(command, "add") == 0) {
            add_command(args, count_args);
        }

        else if (command != NULL && strcmp(command, "init") == 0) {
            init_repository();
        }

        else if (command != NULL && strcmp(command, "remove") == 0) {
            remove_command(args, count_args);
        }

        else if (command != NULL && strcmp(command, "exit") == 0) {
            break;
        }

        else {
            printf("Unknown command\n");
        }
    }

    return 0;
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
    printf("  remove         Add file contents to the index\n");
    printf("  exit           Exit the program\n\n");
}