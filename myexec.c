#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>

#define DEBUG 0

int main(int argc, char* const argv[], char* const envp[]) {
  pid_t pid;
  int status;
  //argv[1] : command name, argv[2..] : arguments
  if (argc <= 1) {
    fprintf(stderr, "argc <= 1\n");
    return 1;
  }
  pid = fork();
#if DEBUG
  fprintf(stderr, "pid = %d\n", pid);
#endif
  if (pid == 0) { //child
    int result = execve(argv[1], argv + 1, envp);
#if DEBUG
    fprintf(stderr, "result = %d\n", result);
#endif
    return result;
  }
  //parent, pid is the id of children
  wait(&status);
#if DEBUG
  fprintf(stderr, "status = %d\n", status);
#endif
  return 0;
}

