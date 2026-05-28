#ifndef INDEX_H
#define INDEX_H

// Добавить/обновить запись в index
void index_update(const char *status, const char *file_name, const char *hash);

// Основные команды
void add_command(char **args, int count);
void remove_command(char **args, int count);

void add_file(char *real_path, char *repo_path);
void remove_file(char *repo_path);

#endif