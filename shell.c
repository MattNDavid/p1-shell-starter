#include "shell.h"

int main(int argc, char **argv) {
  if (argc == 2 && equal(argv[1], "--interactive")) {
    return interactiveShell();
  } else {
    return runTests();
  }
}

// interactive shell to process commands
int interactiveShell() {
  bool should_run = true;
  char *line = calloc(1, MAXLINE);
  while (should_run) {
    printf(PROMPT);
    fflush(stdout);
    int n = fetchline(&line);
    printf("read: %s (length = %d)\n", line, n);
    // ^D results in n == -1
    if (n == -1 || equal(line, "exit")) {
      should_run = false;
      continue;
    }
    if (equal(line, "")) {
      continue;
    }
    processLine(line);
  }
  free(line);
  return 0;
}
void processCommand(char *cmd, bool ampersand) {
  char *commands[100];
  int index = 0;

  char *token = strtok(cmd, " ");
  while (token != NULL) {
    commands[index++] = token;
    token = strtok(NULL, " ");
  }
  commands[index] = NULL;

  pid_t pid = fork();
  if (pid == 0) {
    // Child process
    execvp(commands[0], commands);
    perror("execvp failed");
    _exit(1);
  } else if (pid > 0) {
    // Parent process
    if (!ampersand) {
      int status;
      waitpid(pid, &status, 0);
    } else {
      printf("Running %s in the background\n", commands[0]);
    }
  } else {
    perror("fork failed");
  }
}

void processLine(char *line) {

  printf("processing line: %s\n", line);

  char *command = strtok(line, ";");
  while (command != NULL) {
    // Check if the command should run in the background
    bool ampersand = false;
    if (command[strlen(command) - 1] == '&') {
      ampersand = true;
      command[3] = '\0'; // Remove the & character
    }
    for(int i = 0; i < strlen(command); i++) {
      printf("command[%d] = %c\n", i, command[i]);
    }
    processCommand(command, ampersand);

    command = strtok(NULL, ";");
  }
}

int runTests() {
  printf("*** Running basic tests ***\n");
  char lines[7][MAXLINE] = {
      "ls",      "ls -al", "ls & whoami ;", "ls > junk.txt", "cat < junk.txt",
      "ls | wc", "ascii"};
  for (int i = 0; i < 7; i++) {
    printf("* %d. Testing %s *\n", i + 1, lines[i]);
    processLine(lines[i]);
  }

  return 0;
}

// return true if C-strings are equal
bool equal(char *a, char *b) { return (strcmp(a, b) == 0); }

// read a line from console
// return length of line read or -1 if failed to read
// removes the \n on the line read
int fetchline(char **line) {
  size_t len = 0;
  size_t n = getline(line, &len, stdin);
  if (n > 0) {
    (*line)[n - 1] = '\0';
  }
  return n;
}
