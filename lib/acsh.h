#ifndef _ASCH_H
#define _ASCH_H 1

#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define SHELL "acsh"
#define BUFSIZE 64

typedef struct {
  // Name of shell: ACSH
  char *shell;

  // Present working directory
  char pwd[PATH_MAX];

  // Prompt to display after every command
  char *prompt;

  // User name
  char *username;

  // Hostname
  char *hostname;
} shell_state;

void main_loop(void);

// Reads line from stdin
char *fetch_line(void);

/* Split line into tokens stored in args

On success: Returns number of tokens

On Failure: Returns -1

Returns 0 if line is empty
*/
int split_line(char *line, char ***args);

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

// Shell builtin: which
int shell_which(char **args);

// Handles signals
void signal_handler(int signo);
#endif /* ASCH_H */
