#ifndef EXIST_DIR_H
#define EXIST_DIR_H

#include <stdbool.h>

// Проверяет, существует ли директория
bool directory_exists(const char *path);

// Проверяет, существует ли обычный файл
bool file_exists(const char *path);

// Проверяет, существует ли файл или директория
bool path_exists(const char *path);

#endif