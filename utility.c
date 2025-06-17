#include "myshell.h"

void clear_screen() {
    printf("\033[H\033[J");
}

void list_directory(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("dir");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        printf("%s\n", entry->d_name);
    }

    closedir(dir);
}

