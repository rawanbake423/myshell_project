#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <ctype.h>
#include "myshell.h"
#define PATH_MAX 4096
extern char **environ;

void trim_newline(char *line) {
    size_t len = strlen(line);
    if (len > 0 && line[len - 1] == '\n')
        line[len - 1] = '\0';
}

void parse_input(char *line, char **args, int *background, char **input_file, char **output_file, int *append) {
    *background = 0;
    *input_file = NULL;
    *output_file = NULL;
    *append = 0;
    int i = 0;
    char *token = strtok(line, " ");
    while (token != NULL) {
        if (strcmp(token, "<") == 0) {
            token = strtok(NULL, " ");
            *input_file = token;
        } else if (strcmp(token, ">") == 0) {
            token = strtok(NULL, " ");
            *output_file = token;
        } else if (strcmp(token, ">>") == 0) {
            token = strtok(NULL, " ");
            *output_file = token;
            *append = 1;
        } else if (strcmp(token, "&") == 0) {
            *background = 1;
        } else {
            args[i++] = token;
        }
        token = strtok(NULL, " ");
    }
    args[i] = NULL;
}

int is_internal_command(const char *cmd) {
    return strcmp(cmd, "clr") == 0 || strcmp(cmd, "dir") == 0 || strcmp(cmd, "environ") == 0 ||
           strcmp(cmd, "echo") == 0 || strcmp(cmd, "help") == 0 || strcmp(cmd, "pause") == 0 ||
           strcmp(cmd, "cd") == 0 || strcmp(cmd, "pwd") == 0 || strcmp(cmd, "quit") == 0;
}

void handle_internal_command(char **args, int in, int out, char *input_file, char *output_file, int append) {
    int saved_in = dup(STDIN_FILENO);
    int saved_out = dup(STDOUT_FILENO);
    int fd;

    if (input_file) {
        in = open(input_file, O_RDONLY);
        dup2(in, STDIN_FILENO);
        close(in);
    }
    if (output_file) {
        fd = open(output_file, append ? O_WRONLY | O_CREAT | O_APPEND : O_WRONLY | O_CREAT | O_TRUNC, 0644);
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }

    if (strcmp(args[0], "clr") == 0) {
        clear_screen();
    } else if (strcmp(args[0], "dir") == 0) {
        if (args[1])
            execlp("ls", "ls", "-al", args[1], NULL);
        else
            execlp("ls", "ls", "-al", ".", NULL);
        perror("ls");
    } else if (strcmp(args[0], "environ") == 0) {
        for (char **env = environ; *env; ++env)
            printf("%s\n", *env);
    } else if (strcmp(args[0], "echo") == 0) {
        for (int i = 1; args[i]; ++i)
            printf("%s ", args[i]);
        printf("\n");
    } else if (strcmp(args[0], "help") == 0) {
        printf("Help: Built-in commands are: clr, dir, environ, echo, help, pause, cd, pwd, quit\n");
    } else if (strcmp(args[0], "pause") == 0) {
        printf("Press Enter to continue...");
        getchar();
    } else if (strcmp(args[0], "cd") == 0) {
        if (args[1]) chdir(args[1]);
    } else if (strcmp(args[0], "pwd") == 0) {
        char cwd[MAX_PATH];
        getcwd(cwd, sizeof(cwd));
        printf("%s\n", cwd);
    } else if (strcmp(args[0], "quit") == 0) {
        exit(0);
    }

    dup2(saved_in, STDIN_FILENO);
    dup2(saved_out, STDOUT_FILENO);
    close(saved_in);
    close(saved_out);
}

void execute_command(char *line) {
    char *args[MAX_ARGS];
    int background = 0, append = 0;
    char *input_file = NULL, *output_file = NULL;

    trim_newline(line);
    if (strlen(line) == 0) return;

    parse_input(line, args, &background, &input_file, &output_file, &append);
    if (!args[0]) return;

    if (is_internal_command(args[0])) {
        handle_internal_command(args, 0, 1, input_file, output_file, append);
    } else {
        pid_t pid = fork();
        if (pid == 0) {
            int in, out;
            if (input_file) {
                in = open(input_file, O_RDONLY);
                dup2(in, STDIN_FILENO);
                close(in);
            }
            if (output_file) {
                out = open(output_file, append ? O_WRONLY | O_CREAT | O_APPEND : O_WRONLY | O_CREAT | O_TRUNC, 0644);
                dup2(out, STDOUT_FILENO);
                close(out);
            }
            execvp(args[0], args);
            perror("exec");
            exit(1);
        } else {
            if (!background) waitpid(pid, NULL, 0);
        }
    }
}

void read_batch_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening batch file");
        return;
    }

    char line[MAX_LINE];
    while (fgets(line, sizeof(line), file)) {
        execute_command(line);
    }
    fclose(file);
}

void init_shell_environment() {
    char pathbuf[PATH_MAX];
    getcwd(pathbuf, sizeof(pathbuf));

    char shell_path[PATH_MAX];
    snprintf(shell_path, PATH_MAX - 1, "%s/myshell", pathbuf);
    shell_path[PATH_MAX - 1] = '\0';
    setenv("shell", shell_path, 1);
}

int main(int argc, char *argv[]) {
    init_shell_environment();
    printf("\n\n\n\t+++++++++++++++ OUR SHELL +++++++++++++++\n");
    printf("\t\t++++++++ rawan , alaa , shahed ++++++++\n");
    printf("\t....Type 'help' for entire user manual\n");
    printf("to show the information about the command you want...\n\n\n");

    if (argc == 2) {
        read_batch_file(argv[1]);
        return 0;
    }

    char line[MAX_LINE];
    while (1) {
        printf("MyShell> ");
        if (!fgets(line, MAX_LINE, stdin)) break;
        execute_command(line);
    }

    return 0;
}
