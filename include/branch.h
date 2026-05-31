#ifndef BRANCH_H
#define BRANCH_H

void branch_command(char **args, int count);
char* get_current_branch_name();
char* get_branch_commit(const char *branch_name);
void update_branch_head(const char *branch_name, const char *commit_hash);

#endif