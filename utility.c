// utility.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include "myshell.h"

void handle_cd(char **args) {
    static char prev_dir[1024] = "";

    char *target = args[1];
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));

    if (target == NULL) {
        target = getenv("HOME");  // بدون مسار يعني نرجع إلى $HOME
    } else if (strcmp(target, "-") == 0) {
        if (prev_dir[0] == '\0') {
            printf("cd: OLDPWD not set\n");
            return;
        } else {
            target = prev_dir;
        }
    } else if (target[0] == '~') {
        const char *home = getenv("HOME");
        if (home) {
            static char resolved[1024];
            snprintf(resolved, sizeof(resolved), "%s%s", home, target + 1);
            target = resolved;
        }
    }

    // الآن نحاول تغيير المجلد
    if (chdir(target) != 0) {
        perror("cd");
    } else {
        strncpy(prev_dir, cwd, sizeof(prev_dir));
    }
}


void handle_clr() {
    // تنظيف الشاشة
    printf("\033[H\033[J");
}

void handle_dir(char **args) {
    DIR *d;
    struct dirent *dir;
    char resolved_path[1024];

    // المسار الافتراضي هو current directory "."
    char *directory = args[1] ? args[1] : ".";

    // دعم تحويل ~ إلى مسار HOME
    if (directory[0] == '~') {
        const char *home = getenv("HOME");
        if (home) {
            snprintf(resolved_path, sizeof(resolved_path), "%s%s", home, directory + 1);
            directory = resolved_path;
        }
    }

    d = opendir(directory);
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            printf("%s\n", dir->d_name);
        }
        closedir(d);
    } else {
        perror("dir");
    }
}

void handle_environ() {
    extern char **environ;
    for (char **env = environ; *env != NULL; env++) {
        printf("%s\n", *env);
    }
}

void handle_echo(char **args) {
    for (int i = 1; args[i] != NULL; i++) {
        printf("%s ", args[i]);
    }
    printf("\n");
}

void handle_pause() {
    printf("Press Enter to continue...");
    while (getchar() != '\n');
}

void handle_help() {
    // افتراضياً يُفتح ملف readme باستخدام more
    system("more readme");
}

void handle_internal_command(char **args) {
    if (strcmp(args[0], "cd") == 0) handle_cd(args);
    else if (strcmp(args[0], "clr") == 0) handle_clr();
    else if (strcmp(args[0], "dir") == 0) handle_dir(args);
    else if (strcmp(args[0], "environ") == 0) handle_environ();
    else if (strcmp(args[0], "echo") == 0) handle_echo(args);
    else if (strcmp(args[0], "pause") == 0) handle_pause();
    else if (strcmp(args[0], "help") == 0) handle_help();
}

int is_internal_command(char **args) {
    const char *internal[] = {"cd", "clr", "dir", "environ", "echo", "help", "pause", "quit"};
    for (int i = 0; i < 8; i++) {
        if (strcmp(args[0], internal[i]) == 0) return 1;
    }
    return 0;
}

