#include "lib/acsh.h"

int main(int argc, char **argv) {
  // Start program in main loop
  main_loop();
  return EXIT_SUCCESS;
}

void main_loop() {
  char *line;
  char **args;
  int status;
  int numtokens;
  shell_state state;
  state.shell = SHELL;
  state.prompt = "username@hostname";

  // Get present working directory
  if (getcwd(state.pwd, PATH_MAX) == NULL) {
    perror(SHELL);
    exit(EXIT_FAILURE);
  }

  signal(SIGINT, SIG_IGN);
  do {
    printf("%s (%s)> ", state.prompt, state.pwd);
    line = fetch_line();
    char **args;
    if ((numtokens = split_line(line, &args)) > 0) {
      status = execute_cmd(args);
      free(args);
    } else if (numtokens == 0) {
      status = 1;
      continue;
    } else {
      exit(EXIT_FAILURE);
    }

    free(line);
  } while (status);
}

char *fetch_line() {
  // Read line from stdin
  char *line = NULL;
  size_t buffer, size;

  buffer = getline(&line, &size, stdin);

  return line;
}

int split_line(char *line, char ***tokens) {
  char *delimiters = " \t\n\r\a";
  char *token;
  int numtokens;

  if (tokens == NULL) {
    errno = EINVAL;
    fprintf(stderr, "%s: Invalid pointer for args passed\n", SHELL);
    return -1;
  }
  // Split line into tokens

  /* count the number of tokens in line */
  char *dup = strdup(line);
  numtokens = 0;
  if (strtok(dup, delimiters) != NULL)
    for (numtokens = 1; strtok(NULL, delimiters) != NULL; ++numtokens)
      ;
  free(dup);

  *tokens = malloc(sizeof(char *) * (numtokens + 1));
  if (!(*tokens)) {
    fprintf(stderr, "%s: allocation error", SHELL);
    perror(SHELL);
    return -1;
  }

  if (numtokens == 0) {
    free(*tokens);
    return 0;
  } else {
    *(*tokens) = strtok(line, delimiters);
    for (int i = 1; i < numtokens; ++i)
      *(*(tokens) + i) = strtok(NULL, delimiters);
  }
  (*tokens)[numtokens] = NULL;

  return numtokens;
}

int launch_program(char **args) {
  pid_t pid, wpid;
  int status;
  pid = fork();
  if (pid == 0) {
    signal(SIGINT, SIG_DFL);
    if (execvp(args[0], args) == -1) {
      fprintf(stderr, "%s: command not found: %s\n", SHELL, args[0]);
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    perror(SHELL);
  } else {
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

char *builtin_str[] = {"cd", "help", "exit", "pwd", "which"};

int (*builtin_func[])(char **) = {
    &shell_cd, &shell_help, &shell_exit, &shell_pwd, &shell_which,
};

// Return number of shell builtin commands
int shell_num_builtins() { return sizeof(builtin_str) / sizeof(char *); }

int shell_cd(char **args) {
  const char *path;
  if (args[1] == NULL) {
    // Set path as user home directory if no path is provided
    if ((path = getenv("HOME")) == NULL) {
      path = getpwuid(getuid())->pw_dir;
    }
  } else {
    path = args[1];
  }

  if (chdir(path) != 0) {
    perror(SHELL);
  }
  return 1;
}

int shell_help(char **args) {
  printf("\n\tEzejim Amblessed's ACSH\n");
  printf("Builtin functions include: \n");

  for (int i = 0; i < shell_num_builtins(); ++i) {
    printf(" %s\n", builtin_str[i]);
  }

  printf("Use the man command to get information on other progrms\n");

  return 1;
}

int shell_pwd(char **args) {
  char cwd[PATH_MAX];

  if (getcwd(cwd, sizeof(cwd)) != NULL) {
    printf("%s\n", cwd);
  } else {
    perror(SHELL);
  }

  return 1;
}

int shell_which(char **args) {
  int i;
  const char *command;
  int status;

  if (args[1] == NULL) {
    fprintf(stderr, "%s: Usage of which: which <command>\n", SHELL);
    return 1;
  }

  for (i = 0; i < shell_num_builtins(); ++i) {
    if (strcmp(args[1], builtin_str[i]) == 0) {
      printf("%s: shell built-in command", args[1]);
      return 1;
    }
  }

  // Call the default which command if argument 1 (command to search for) is not
  // a shell builtin command
  status = launch_program(args);
  return status;
}

int shell_exit(char **args) { return 0; }

int execute_cmd(char **args) {
  int i;
  if (args[0] == NULL) {
    return 1;
  }

  for (i = 0; i < shell_num_builtins(); ++i) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  int status = launch_program(args);
  return status;
}
