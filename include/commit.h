#ifndef COMMIT_H
#define COMMIT_H

void commit_command(char **args, int count);
void create_initial_commit();
char* get_head_commit();

// сохранить commit object
void save_commit_object(const char *hash, const char *content);

// обновить branch
void update_current_branch(const char *hash);

// очистить index
void clear_index();

char* get_last_commit_hash(const char *filename);

#endif