#ifndef MYSHELL_H
#define MYSHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <ctype.h>
#include <dirent.h>

#define MAX_LINE 1024
#define MAX_ARGS 64

#ifndef MAX_PATH
#define MAX_PATH 1024
#endif

void clear_screen();
void list_directory(const char *path);

#endif
