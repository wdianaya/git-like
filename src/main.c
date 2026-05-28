#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "repository.h"
#include "utils.h"
#include "object.h"
#include "index.h"
#include "commit.h"
#include "log.h"

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

        char* command = NULL;
        char* args[MAX_ARGS];
        int count_args = 0;

        char buff_copy[MAX_INPUT];
        snprintf(buff_copy, sizeof(buff_copy), "%s", buff);
        parse_argument(buff_copy, &command, args, &count_args);

        if (command == NULL) {
            fprintf(stderr, "Unknown command\n");
        }

        else if (strcmp(command, "help") == 0) {
            show_help();
        }
        
        else if (strcmp(command, "add") == 0) {
            add_command(args, count_args);
        }

        else if (strcmp(command, "init") == 0) {
            init_repository();
        }

        else if (strcmp(command, "remove") == 0) {
            remove_command(args, count_args);
        }

        else if (strcmp(command, "exit") == 0) {
            break;
        }

        else if (strcmp(command, "commit") == 0) {
            if (count_args == 0) {
                printf("commit message required\n");
            } else {
                commit_command(args, count_args);
            }
        }

        else if (strcmp(command, "log") == 0) {
            log_command(args, count_args);
        }

        else {
            fprintf(stderr, "Unknown command\n");
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
    printf("  init           Initialize a new repository in current directory\n");
    printf("  help           Show this help message\n");
    printf("  add            Add file contents to the index\n");
    printf("  remove         Add file contents to the index\n");
    printf("  commit         Record changes to the repository\n");
    printf("  log [hash] [[--n] <number>]    Show commit logs"\n);
    printf("  exit           Exit the program\n\n");
}