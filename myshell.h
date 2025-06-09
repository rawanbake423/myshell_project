// myshell.h
#ifndef MYSHELL_H
#define MYSHELL_H

// تعريف الحد الأقصى للأمر
#define MAX_COMMAND_SIZE 1024
#define MAX_TOKENS 100

// الدوال المستخدمة
void execute_command(char **args, int background);
int is_internal_command(char **args);
void handle_internal_command(char **args);
void redirect_input_output(char **args, int *in, int *out, char **input_file, char **output_file, int *append);
void remove_redirection_tokens(char **args);
void read_batch_file(const char *filename);
void display_prompt();
void trim_whitespace(char *str);
void parse_command(char *line, char **args, int *background);
void init_shell_environment();

#endif

