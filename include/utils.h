#ifndef EXIST_DIR_H
#define EXIST_DIR_H

#include <stdbool.h>

// Проверяет, существует ли директория
bool directory_exists(const char *path);

// Проверяет, существует ли обычный файл
bool file_exists(const char *path);

// Проверяет, существует ли объект по данному пути
bool path_exists(const char *path);

// Получить абсолютный путь переданного объекта
void get_repo_path(char *name, char **path);

// Записывает путь по хэшу
void build_object_path(const char *hash, char *path);

// Определяет branch or commit
char* resolve_commit(const char *name);

// Вернет 1 если detached HEAD
int is_detached_head(void);
#endif