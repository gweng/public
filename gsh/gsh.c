#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#ifdef DEBUG
  #define TRACE(p...) printf(p)
#else
  #define TRACE(p...)
#endif

#define STR_DELIM   " \t\n\a\r"
#define BUFFER_SIZE 64

static int last_status = 0;

char **split_line(char *line, int *argc) {

  size_t len = BUFFER_SIZE;
  char **argv = malloc(len * sizeof(char *));

  if (!argv) {
    perror("failed to allocate buffer");
    exit(1);
  }

  int pos = 0;
  char *arg = strtok(line, STR_DELIM);
  while (arg != NULL) {

    argv[pos++] = arg;

    if (pos >= len) {

      len += BUFFER_SIZE;
      argv = realloc(argv, len * sizeof(char *));

      if (!argv) {
        perror("failed to allocated buffer");
        exit(1);
      }
    }

    arg = strtok(NULL, STR_DELIM);
  }
  argv[pos] = NULL;
  *argc = pos; 
  return argv;
}

void run_command(char **argv, int in, int out) {
  TRACE("in=%d out=%d\n", in, out);
  pid_t pid = fork();

  if (pid == 0) {
    // child
    TRACE("forked pid=%d\n", pid);
    if (in > 0) {
      dup2(in, 0);
      close(in);
    }
    if (out > 0) {
      dup2(out, 1);
      close(out);
    }
    char *cmd = argv[0];
    if (execvp(cmd, argv) < 0) {
      perror("exec error");
    }
    exit(1);
  } else if (pid > 0) {
    // parent
    if (in > 0) {
      close(in);
    }
    if (out > 0) {
      close(out);
    }
    pid = wait(&last_status);
    TRACE("reaped pid=%d\n", pid);
  } else {
    perror("fork error");
  }
}

void run_cmds(char **split, const int argc) {

  TRACE("run_cmds\n");
  char **argv = split;
  int p = 0;
  int pipes[argc][2];

  for (int i = 0; split[i] != NULL; ++i) {
    TRACE("arg=%s\n", split[i]);
    int in  = p == 0 ? -1 : pipes[p-1][0];
    int out = -1;

    if (*split[i] == '|') {
      split[i] = NULL;
      pipe(pipes[p]);

      out = pipes[p++][1];
      run_command(argv, in, out);
      argv = &split[i+1];
    }
  }

  TRACE("cmd=%s\n", argv[0]);
  int out = p == 0 ? -1 : pipes[p-1][0];
  run_command(argv, out, -1);
}

void read_line(char **line, size_t *len) {

  ssize_t bytes = getline(line, len, stdin);

  if (bytes == -1) {
    if (feof(stdin) || strcmp("exit", *line)) {
      free(*line);
      exit(0);
    } else {
      perror("Error while reading line");
      exit(1);
    }
  }
}

void run() {

  while (1) {

    printf("$ ");

    char    *l   = NULL;
    size_t  size = 0;

    read_line(&l, &size);
    int argc;
    char **split = split_line(l, &argc);
    run_cmds(split, argc);

    free(l);
    free(split);
  }
}

int main() {

  run();
  return 0;
}
