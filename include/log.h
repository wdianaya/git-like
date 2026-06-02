#ifndef LOG_H
#define LOG_H

void log_command(char **args, int count);

typedef struct CommitInfo {
    char hash[41];
    char parent[41];
    char date[64];
    char message[1024];
} CommitInfo;

#endif