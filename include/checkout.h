#ifndef CHECKOUT_H
#define CHECKOUT_H

int find_file_in_commit(const char *commit_hash, const char *filename, char *blob_hash);
int restore_blob(const char *blob_hash, const char *filename);
void checkout_command(char **args, int count);

#endif
