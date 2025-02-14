#include "acsh.h"

int main(int argc, char **argv) {
  // Start program in main loop
  main_loop();
  return EXIT_SUCCESS;
}

void main_loop() {
  char *line;
  char **args;
  int status;
  do {
    printf("%s> ", SHELL);
    line = fetch_line();
    args = split_line(line);
    status = execute_cmd(args);

    free(line);
    free(args);
  } while (status);
}

char *fetch_line() {
  char *input = NULL;
  size_t buffer;

  if (getline(&input, &buffer, stdin) == -1) {
    if (feof(stdin)) {
      exit(EXIT_SUCCESS);
    } else {
      perror("readline");
      exit(EXIT_FAILURE);
    }
  }

  return input;
}

char **split_line(char *line) {

  int bufsize = BUFSIZE, position = 0;
  char *delimiters = " \t\n\r\a";
  char **tokens = malloc(sizeof(char *) * bufsize);
  char *token;

  if (!tokens) {
    fprintf(stderr, "%s: allocation error", SHELL);
    exit(EXIT_FAILURE);
  }

  token = strtok(line, delimiters);
  while (token != NULL) {
    tokens[position] = token;
    ++position;
    if (position >= bufsize) {
      bufsize += BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char *));

      if (!tokens) {
        fprintf(stderr, "%s: allocation error", SHELL);
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, delimiters);
  }

  return tokens;
}

int launch_program(char **args) {
  pid_t pid, wpid;
  int status;
  pid = fork();
  if (pid == 0) {
    if (execvp(args[0], args) == -1) {
      perror(SHELL);
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

char *builtin_str[] = {"cd", "help", "exit", "pwd"};

int (*builtin_func[])(char **) = {
    &shell_cd,
    &shell_help,
    &shell_exit,
    &shell_pwd,
};

int shell_num_builtins() { return sizeof(builtin_str) / sizeof(char *); }

int shell_cd(char **args) {
  if (args[1] == NULL) {
    fprintf(stderr, "%s: expected arguments to \"cd\"\n", SHELL);
  } else {
    if (chdir(args[1]) != 0) {
      perror(SHELL);
    }
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
