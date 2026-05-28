#ifndef DIFF_H
#define DIFF_H

typedef struct FileEntry {
    char status[2];
    char filename[512];
    char hash[41];
} FileEntry;

void diff_command(char **args, int count);

#endif