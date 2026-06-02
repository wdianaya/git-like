#ifndef INDEX_H
#define INDEX_H

int file_in_index(const char *filename);
void index_update(const char *status, const char *file_name, const char *hash);
char* get_file_hash_from_head(const char *filename);
void stage_file(const char *real_path, const char *repo_path);
void add_directory(const char *real_dir, const char *repo_dir);
void detect_deleted_files(const char *prefix);
void add_command(char **args, int count);

#endif
