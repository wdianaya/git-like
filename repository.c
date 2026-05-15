#include <stdio.h>
#include <stdlib.h>      
#include <sys/stat.h> 
   
#include "utils.h"

// инициализация репозитория
void init_repository() {
    if (directory_exists(".mygit")) {
        printf("Repository already exist\n");
        return;
    }

    mkdir(".mygit", 0755);
    mkdir(".mygit/objects", 0755);
    mkdir(".mygit/refs", 0755);
    mkdir(".mygit/refs/heads", 0755);

    FILE* head = fopen(".mygit/HEAD", "w");
    fprintf(head, "ref: refs/heads/master\n");
    fclose(head);

    FILE *index = fopen(".mygit/index", "w");
    fclose(index);

    char dirname[] = ".mygit";

    char* path;
    get_repo_path(dirname, &path);
    printf("Initialized empty Git repository in path:");
    if (path == NULL) {
        printf("[cannot find]");
    } else {
        printf("[%s]", path);
        free(path);
    }

    printf("\n");
}