#ifndef COMMIT_H
#define COMMIT_H

typedef struct {
    char hash[41];
    char parent[41];
    char date[64];
    char message[512];
} CommitInfo;

char* get_head_commit(void);
void save_commit_object(const char *hash, const char *content);
void update_current_branch(const char *hash);
void clear_index(void);
void commit_command(char **args, int count);
void create_initial_commit(void);
char* get_last_commit_hash(const char *filename);

#endif
