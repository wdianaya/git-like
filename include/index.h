#ifndef INDEX_H
#define INDEX_H

// Добавить/обновить запись в index
void index_update(const char *status, const char *file_name, const char *hash);

void add_command(char **args, int count);

#endif