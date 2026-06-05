#ifndef DIFF_H
#define DIFF_H

typedef struct {
    char status[3];
    char filename[512];
    char hash[41];
} FileEntry;

int find_file(FileEntry files[], int count, const char *filename);
int load_commit_files(const char *hash, FileEntry files[], int max_files);
void diff_command(char **args, int count);

#endif
