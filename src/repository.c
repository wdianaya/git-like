#include <stdio.h>
#include <stdlib.h>

#include "utils.h"
#include "commit.h"
#include "branch.h"

#ifdef _WIN32
    #include <direct.h>
    #define MKDIR(path) _mkdir(path)
#else
    #include <sys/stat.h>
    #define MKDIR(path) mkdir(path, 0755)
#endif


// инициализация репозитория
void init_repository() {
    if (directory_exists(".mygit")) {
        fprintf(stderr, "Repository already exist\n");
        return;
    }

    MKDIR(".mygit");
    MKDIR(".mygit/objects");
    MKDIR(".mygit/refs");
    MKDIR(".mygit/refs/heads");

    FILE* head = fopen(".mygit/HEAD", "w");
    if (!head) { fprintf(stderr, "cannot create HEAD\n"); return; }
    fprintf(head, "ref: refs/heads/master\n");
    fclose(head);

    FILE *index = fopen(".mygit/index", "w");
    if (index) fclose(index);

    char dirname[] = ".mygit";

    char* path = NULL;
    get_repo_path(dirname, &path);

    fprintf(stderr, "Initialized empty Git repository in path:");

    if (path == NULL) {
        printf("[current directory/.mygit]\n");
    } else {
        printf("[%s]", path);
        free(path);
    }
    printf("\n");

    create_initial_commit();
    char *initial_commit = get_head_commit();
    if (initial_commit) {
        update_branch_head("master", initial_commit);
        free(initial_commit);
    }
}