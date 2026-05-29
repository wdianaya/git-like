#include <stdio.h>
#include <stdlib.h>

#include "utils.h"
#include "commit.h"

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
    fprintf(head, "ref: refs/heads/master\n");
    fclose(head);

    FILE *index = fopen(".mygit/index", "w");
    fclose(index);

    char dirname[] = ".mygit";

    char* path;
    get_repo_path(dirname, &path);

    fprintf(stderr, "Initialized empty Git repository in path:");

    if (path == NULL) {
        fprintf(stderr, "[cannot find]");
    } else {
        printf("[%s]", path);
        free(path);
    }
    printf("\n");

    create_initial_commit();
}