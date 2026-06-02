#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "diff.h"
#include "commit.h"
#include "utils.h"

#define MAX_FILES 1024
#define MAX_LINES 2000
#define MAX_LINE_LEN 4096

typedef struct {
    char **lines;
    int count;
} LineArray;

static LineArray read_lines_from_blob(const char *hash) {
    LineArray la = {NULL, 0};
    char path[512];
    build_object_path(hash, path);

    FILE *f = fopen(path, "r");
    if (!f) return la;

    la.lines = (char**)malloc(MAX_LINES * sizeof(char*));
    char line[MAX_LINE_LEN];

    while (la.count < MAX_LINES && fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\n")] = '\0';
        la.lines[la.count++] = strdup(line);
    }
    fclose(f);
    return la;
}

static void free_lines(LineArray *la) {
    for (int i = 0; i < la->count; i++) {
        free(la->lines[i]);
    }
    free(la->lines);
    la->lines = NULL;
    la->count = 0;
}

// lines from hash1 "-", lines from hash2 "+"
static void print_line_diff(const char *hash1, const char *hash2) {
    if (strcmp(hash1, hash2) == 0) return;

    LineArray a = read_lines_from_blob(hash1);
    LineArray b = read_lines_from_blob(hash2);
    
    int n = a.count, m = b.count;

    if (n == 0 && m == 0) {
        free_lines(&a);
        free_lines(&b);
        return; 
    }

    int **dp = (int**)malloc((n + 1) * sizeof(int*));
    for (int i = 0; i <= n; i++)
        dp[i] = (int*)calloc(m + 1, sizeof(int));

    for (int i = 1; i <= n; i++)
        for (int j = 1; j <= m; j++)
            dp[i][j] = (strcmp(a.lines[i- 1], b.lines[j -1]) == 0) ? dp[i-1][j-1] + 1 : (dp[i-1][j] > dp[i][j-1] ? dp[i-1][j] : dp[i][j-1]);

    typedef struct {
        int type;
        int idx;
    } DiffLine; // type: 0= common, 1 =del, 2=add 
    DiffLine *diff = (DiffLine*)malloc((n + m + 1) * sizeof(DiffLine));
    
    int dcount = 0;

    int i = n, j = m;
    while (i > 0 || j > 0) {

        if (i > 0 && j > 0 && strcmp(a.lines[i-1], b.lines[j-1]) == 0) {

            diff[dcount++] = (DiffLine){0, i-1};
            i--;
            j--;
        } else if (j > 0 && (i == 0 || dp[i][j-1] >= dp[i-1][j])) {

            diff[dcount++] = (DiffLine){2, j-1};
            j--;
        } else {
            diff[dcount++] = (DiffLine){1, i-1};
            i--;
        }
    }

    
    for (int k = dcount - 1; k >= 0; k--) {
        if (diff[k].type == 1)
            printf("  - %s\n", a.lines[diff[k].idx]);
        else if (diff[k].type == 2)
            printf("  + %s\n", b.lines[diff[k].idx]);
    }

    free(diff);
    for (int x = 0; x <= n; x++) free(dp[x]);
    free(dp);
    free_lines(&a);
    free_lines(&b);
}
int find_file(FileEntry files[], int count, const char *filename) {
    for (int i = 0; i < count; ++i) {
        if (strcmp(files[i].filename, filename) == 0) {
            return i;
        }
    }
    return -1;
}


int load_commit_files(const char *hash, FileEntry files[], int max_files) {
    char path[512];
    build_object_path(hash, path);

    FILE *f = fopen(path, "r");
    if (!f) return 0;

    char line[1024];
    int count = 0;

    while (fgets(line, sizeof(line), f)) {
        // пропускаем метаданные
        if (strncmp(line, "parent:", 7) == 0 || strncmp(line, "date:", 5) == 0 
            || strncmp(line, "message:", 8) == 0 || strcmp(line, "\n") == 0) {
            continue;
        }

        if (count >= max_files) {
            break;
        }

        sscanf(line, "%1s %511s %40s", files[count].status, files[count].filename, files[count].hash);
        count++;
    }

    fclose(f);
    return count;
}

static void do_diff(const char *hash_a, const char *hash_b, int show_lines) {
    FileEntry files_a[MAX_FILES];
    FileEntry files_b[MAX_FILES];

    int cnt_a = load_commit_files(hash_a, files_a, MAX_FILES);
    int cnt_b = load_commit_files(hash_b, files_b, MAX_FILES);

    printf("\nDiff: %s..%s\n\n", hash_a, hash_b);

    // from A
    for (int i = 0; i < cnt_a; ++i) {
        int idx = find_file(files_b, cnt_b, files_a[i].filename);
        if (idx == -1) {

            printf("ADDED in second: %s  [%s]\n", files_a[i].filename, files_a[i].hash);
        } else if (strcmp(files_a[i].hash, files_b[idx].hash) != 0) {
            printf("MODIFIED: %s\n", files_a[i].filename);
            printf("  commit1: %s\n", files_a[i].hash);

            printf("  commit2: %s\n", files_b[idx].hash);
            if (show_lines) {
                print_line_diff(files_a[i].hash, files_b[idx].hash);
            }
        }
    }

    // from B
    for (int i = 0; i < cnt_b; ++i) {
        int idx = find_file(files_a, cnt_a, files_b[i].filename);

        if (idx == -1) {

            printf("ADDED in first: %s  [%s]\n", files_b[i].filename, files_b[i].hash);
        }
    }
}

void diff_command(char **args, int count) {
    if (!directory_exists(".mygit")) {
        fprintf(stderr, "repository not initialized\n");
        return;
    }
    if (count == 0) {
        fprintf(stderr, "usage: diff <commit1|branch1> [commit2|branch2]\n");
        return;
    }

    char *hash_a = resolve_commit(args[0]);
    if (!hash_a) {

        fprintf(stderr, "commit or branch '%s' not found\n", args[0]);
        return;
    }

    char *hash_b = NULL;
    if (count >= 2) {
        hash_b = resolve_commit(args[1]);
        if (!hash_b) {
            fprintf(stderr, "commit or branch '%s' not found\n", args[1]);
            free(hash_a);
            return;
        }

    } else {
        hash_b = get_head_commit();
        if (!hash_b) {

            fprintf(stderr, "no commits yet\n");
            free(hash_a);
            return;
        }
    }

    do_diff(hash_a, hash_b, 1);

    free(hash_a);
    free(hash_b);
}