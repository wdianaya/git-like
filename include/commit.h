#ifndef COMMIT_H
#define COMMIT_H


char* get_head_commit(void);
void save_commit_object(const char *hash, const char *content);
void update_current_branch(const char *hash);
void clear_index(void);
void commit_command(char **args, int count);
void create_initial_commit(void);
char* get_last_commit_hash(const char *filename);

#endif
