#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "commit.h"
#include "utils.h"
#include "log.h"

int read_commit(const char *hash, CommitInfo *commit) {
    char path[512];

    build_object_path(hash, path);
    FILE *f = fopen(path, "r");

    if (!f) return 0;
    strcpy(commit->hash, hash);
    char line[1024];

    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "parent:", 7) == 0) {
            sscanf(line,
                   "parent: %40s",
                   commit->parent);
        }
        else if (strncmp(line, "date:", 5) == 0) {
            sscanf(line,
                   "date: %[^\n]",
                   commit->date);
        }
        else if (strncmp(line, "message:", 8) == 0) {
            sscanf(line,
                   "message: %[^\n]",
                   commit->message);
        }
    }

    fclose(f);

    return 1;
}

void log_command(char **args, int count) {
    char *start_commit = NULL;
    int limit = INT_MAX;

    // parse args
    switch(count) {
        case 0:
            break;
        case 1:
            start_commit = args[0];
            break;
        case 2:
            if (strcmp(args[0], "--n") == 0) {
                limit = atoi(args[1]);
            } else {
                fprintf(stderr, "uncorrect arguments passed\n");
                return;
            }
            break;
        case 3:
            if (strcmp(args[1], "--n") == 0) {
                start_commit = args[0];
                limit = atoi(args[2]);
            } else {
                fprintf(stderr, "uncorrect arguments passed\n");
                return;
            }
            break;
        default:
            fprintf(stderr, "many arguments passed\n");
            return;
    }

    // если commit не указан
    if (!start_commit) {
        start_commit = get_head_commit();
        if (!start_commit) {
            printf("No commits\n");
            return;
        }
    }

    char current_hash[41];
    strcpy(current_hash, start_commit);
    int printed = 0;

    while (strlen(current_hash) > 0 && strcmp(current_hash, "NULL") != 0 && printed < limit) {
        CommitInfo commit;

        if (!read_commit(current_hash, &commit)) {
            break;
        }

        printf("\ncommit %s\n", commit.hash);
        printf("Date: %s\n", commit.date);
        printf("Message: %s\n", commit.message);
        strcpy(current_hash, commit.parent);

        printed++;
    }
}