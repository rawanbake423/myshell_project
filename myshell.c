// myshell.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "myshell.h"

// عرض المسار الحالي في prompt
void display_prompt() {
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("%s> ", cwd);
    fflush(stdout);
}

// حذف الفراغات الزائدة في البداية والنهاية
void trim_whitespace(char *str) {
    char *end;
    while (*str == ' ' || *str == '\t') str++;
    end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n')) end--;
    *(end + 1) = '\0';
}

// تحليل الأمر إلى كلمات مفصولة
void parse_command(char *line, char **args, int *background) {
    *background = 0;
    int i = 0;
    char *token = strtok(line, " \t");
    while (token != NULL) {
        if (strcmp(token, "&") == 0) {
            *background = 1;
        } else {
            args[i++] = token;
        }
        token = strtok(NULL, " \t");
    }
    args[i] = NULL;
}

// إعادة توجيه الإدخال والإخراج
void redirect_input_output(char **args, int *in, int *out, char **input_file, char **output_file, int *append) {
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "<") == 0 && args[i+1] != NULL) {
            *in = 1;
            *input_file = args[i+1];
            args[i] = NULL;
        } else if ((strcmp(args[i], ">") == 0 || strcmp(args[i], ">>") == 0) && args[i+1] != NULL) {
            *out = 1;
            *append = (strcmp(args[i], ">>") == 0);
            *output_file = args[i+1];
            args[i] = NULL;
        }
    }
}

// تنفيذ الأمر الخارجي
void execute_command(char **args, int background) {
    pid_t pid;
    int in = 0, out = 0, append = 0;
    char *input_file = NULL, *output_file = NULL;

    redirect_input_output(args, &in, &out, &input_file, &output_file, &append);

    pid = fork();
    if (pid == 0) {
        // عملية الطفل

        // إدخال
        if (in && input_file) {
            int fd = open(input_file, O_RDONLY);
            if (fd < 0) {
                perror("Input redirection");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        // إخراج
        if (out && output_file) {
            int fd = open(output_file, append ? O_WRONLY | O_CREAT | O_APPEND : O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                perror("Output redirection");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        // إعداد متغير البيئة
        char pathbuf[1024];
        char shell_path[1024];
        getcwd(pathbuf, sizeof(pathbuf));
        if (strlen(pathbuf) + strlen("/myshell") < sizeof(shell_path)) {
            snprintf(shell_path, sizeof(shell_path), "%s/myshell", pathbuf);
            setenv("parent", shell_path, 1);
        } else {
            fprintf(stderr, "Path too long. Cannot set parent environment variable.\n");
        }

        // تنفيذ الأمر
        if (execvp(args[0], args) == -1) {
            perror("Execution failed");
            exit(EXIT_FAILURE);
        }

    } else if (pid > 0) {
        // العملية الأم
        if (!background) {
            waitpid(pid, NULL, 0);
        }
    } else {
        perror("Fork failed");
    }
}

// تنفيذ ملف batch
void read_batch_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Batch file");
        exit(EXIT_FAILURE);
    }

    char line[MAX_COMMAND_SIZE];
    while (fgets(line, sizeof(line), file)) {
        trim_whitespace(line);
        if (strlen(line) == 0) continue;

        char *args[MAX_TOKENS];
        int background = 0;
        parse_command(line, args, &background);

        if (args[0] == NULL) continue;
        if (strcmp(args[0], "quit") == 0) break;
        if (is_internal_command(args)) {
            handle_internal_command(args);
        } else {
            execute_command(args, background);
        }
    }
    fclose(file);
}

// تهيئة البيئة الخاصة بالشيل
void init_shell_environment() {
    char pathbuf[1024];
    char shell_path[1024];
    getcwd(pathbuf, sizeof(pathbuf));
    if (strlen(pathbuf) + strlen("/myshell") < sizeof(shell_path)) {
        snprintf(shell_path, sizeof(shell_path), "%s/myshell", pathbuf);
        setenv("shell", shell_path, 1);
    } else {
        fprintf(stderr, "Path too long. Cannot set shell environment variable.\n");
    }
}

// البرنامج الرئيسي
int main(int argc, char *argv[]) {
    init_shell_environment();

    // إذا وُجد ملف batch
    if (argc == 2) {
        read_batch_file(argv[1]);
        return 0;
    }

    // الوضع التفاعلي
    char line[MAX_COMMAND_SIZE];
    char *args[MAX_TOKENS];
    int background;

    while (1) {
        display_prompt();

        if (!fgets(line, sizeof(line), stdin)) break;
        trim_whitespace(line);
        if (strlen(line) == 0) continue;

        parse_command(line, args, &background);

        if (args[0] == NULL) continue;
        if (strcmp(args[0], "quit") == 0) break;

        if (is_internal_command(args)) {
            handle_internal_command(args);
        } else {
            execute_command(args, background);
        }
    }

    return 0;
}

