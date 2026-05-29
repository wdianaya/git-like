#include <stdio.h>
#include <string.h>

#include "status.h"
#include "utils.h"

void status_command() {
    if (!directory_exists(".mygit")) {
        fprintf(stderr,"repository not initialized\n");
        return;
    }

    FILE *index = fopen(".mygit/index", "r");

    if (!index) {
        fprintf(stderr,"cannot open index\n");
        return;
    }

    char line[1024];

    int empty = 1;

    while (fgets(line, sizeof(line), index)) {

        char status[2];
        char filename[512];
        char hash[41];

        sscanf(line,
               "%1s %511s %40s",
               status,
               filename,
               hash);

        empty = 0;

        if (strcmp(status, "A") == 0) {
            printf("created: %s\n",filename);
        }

        else if (strcmp(status, "D") == 0) {
            printf("deleted: %s\n",filename);
        }

        else if (strcmp(status, "M") == 0) {
            printf("modified: %s\n", filename);
        }
    }

    fclose(index);

    if (empty) {
        printf("nothing to commit\n");
    }
}