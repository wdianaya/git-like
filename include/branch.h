#ifndef BRANCH_H
#define BRANCH_H

char* get_current_branch_name(void);
char* get_branch_commit(const char *branch_name);
void update_branch_head(const char *branch_name, const char *commit_hash);
void list_branches(void);
void create_branch(const char *branch_name);
void branch_command(char **args, int count);

#endif