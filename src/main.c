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
#include "status.h"
#include "diff.h"
#include "checkout.h"
#include "branch.h"

#define MAX_INPUT 1024
#define MAX_ARGS 64

void parse_argument(char *line, char **func, char **args, int *arg_count);
void show_help();

int main() {
    char buff[MAX_INPUT];
    while (1) {
        printf("mygit> ");
        fflush(stdout);
        if (fgets(buff, sizeof(buff), stdin) == NULL) {
            break;
        }

        buff[strcspn(buff, "\n")] = '\0';
        if (buff[0] == '\0') continue;

        char* command = NULL;
        char* args[MAX_ARGS];
        int count_args = 0;

        char buff_copy[MAX_INPUT];
        strncpy(buff_copy, buff, sizeof(buff_copy) - 1);
        buff_copy[sizeof(buff_copy) - 1] = '\0';
        parse_argument(buff_copy, &command, args, &count_args);

        if (command == NULL) {
            fprintf(stderr, "Unknown command\n");
            continue;
        }
        if (strcmp(command, "help") == 0) {
            show_help();
            continue;
        }
        if (strcmp(command, "exit") == 0) {
            break;
        }
        if (strcmp(command, "init") == 0) {
            init_repository();
            continue;
        }
        if (directory_exists(".mygit") && is_detached_head()) {
            if (strcmp(command, "checkout") != 0 && strcmp(command, "branch")   != 0) {
                fprintf(stderr,"detached HEAD: only 'checkout <commit|branch>' and 'branch [name]' are available\n");
                continue;
            }
        }
        if (strcmp(command, "add") == 0) {
            if (count_args == 0) fprintf(stderr, "usage: add <path> ...\n");
            else add_command(args, count_args);
        }
        else if (strcmp(command, "commit") == 0) {
            if (count_args == 0) fprintf(stderr, "commit message required\n");
            else commit_command(args, count_args);
        }
        else if (strcmp(command, "log") == 0) {
            log_command(args, count_args);
        }
        else if (strcmp(command, "diff") == 0) {
            diff_command(args, count_args);
        }
        else if (strcmp(command, "status") == 0) {
            status_command();
        }
        else if (strcmp(command, "checkout") == 0) {
            checkout_command(args, count_args);
        }
        else if (strcmp(command, "branch") == 0) {
            branch_command(args, count_args);
        }
        else {
            fprintf(stderr, "unknown command '%s' (type 'help' for list)\n", command);
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
    printf("  init                           Initialize a new repository\n");
    printf("  add <path> [...]               Stage files/directories for commit\n");
    printf("  commit <message>               Record staged changes\n");
    printf("  log [commit|branch] [--n <n>]  Show commit history\n");
    printf("  log commit1..commit2           Show range of commits\n");
    printf("  diff <commit1|branch1> [c2]    Diff two commits (line-by-line)\n");
    printf("  status                         Show staged/unstaged/untracked files\n");
    printf("  checkout <commit|branch>       Restore entire repo to commit state\n");
    printf("  checkout <commit|branch> <f>   Restore single file\n");
    printf("  branch                         List all branches\n");
    printf("  branch <name>                  Create new branch at current commit\n");
    printf("  help                           Show this message\n");
    printf("  exit                           Quit\n\n");
}
