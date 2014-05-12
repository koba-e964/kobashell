#include "./kobash.h"

#include <signal.h>

#include <string.h>
#include <unistd.h>
#include "parser/parse.h"
#include <stdio.h>
#include <stdlib.h>

void test_handler(int id) {
  printf("test_handler : %d\n", id);
}


int main(int argc, char *const argv[], char *const envp[]) {
  job *curr_job;
#if DEBUG
  {
    int i = 0;
    char * const * ptr = envp;
    for (; *ptr; ++ptr, ++i) {
      fprintf(stderr, "envp[%d] = \"%s\"\n", i, *ptr);
    }
  }
#endif
  init_path(envp); /* initialization in path.c */
  
  // setting handler
  {
    struct sigaction sa;
    sa.sa_handler = test_handler;
    sa.sa_flags = 0;
    sigfillset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
    sigaction(SIGTSTP, &sa, NULL);
    //sigaction(SIGTTIN, &sa, NULL);
    sa.sa_handler = SIG_IGN;
    sigaction(SIGTTOU, &sa, NULL);
    sa.sa_handler = test_handler;
    sigaction(SIGCHLD, &sa, NULL);
  }

  {
    pid_t group_id;
    group_id = getpid();
    if (setpgid(group_id, group_id) < 0) {
      perror("main.setpgid");
      return 0;
    }
    int ttyfd = 0; // terminal's fd is alleged to be 0!!!
    tcsetpgrp(ttyfd, group_id);
  }
#if DEBUG
  puts("enters while loop:");
#endif
  while (1) {
    char *line;
    kill_defuncts();
#if DEBUG
    puts("reads:\n");
#endif
    line = k_getline("ish$ ");
    if(!strcmp(line, "exit"))
      break;
    curr_job = parse_line(line);
    while (curr_job != NULL) {
      execute_job(curr_job, envp);
      curr_job = curr_job->next;
    }
    free_job(curr_job);
    free(line);
  }
  return 0;
}
