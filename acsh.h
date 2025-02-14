#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define SHELL "acsh"
#define BUFSIZE 64

void main_loop(void);

// Reads line from stdin
char *fetch_line(void);

// Split line returned from readline
char **split_line(char *line);

// Runs commands and return status
int execute_cmd(char **args);

// Launch program with arguments passed into the shell
int launch_program(char **args);

// Shell builtin: cd
int shell_cd(char **args);

// Shell builtin: pwd
int shell_help(char **args);

// Shell builtin: exit
int shell_exit(char **args);

// Display currently working directory
int shell_pwd(char **args);
