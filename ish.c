#include "./kobash.h"

#include <signal.h>

#include <assert.h>
#include <string.h>
#include <unistd.h>
#include "parser/parse.h"
#include <stdio.h>
#include <stdlib.h>


void test_handler(int id) {
#if DEBUG
  printf("test_handler : %d\n", id);
#endif
}

void sigtstp_handler(int id) {
  assert(id == SIGTSTP);
  printf("sigtstp_handler\n");
}

void sigchld_action(int id, siginfo_t *info, void *param) {
#if DEBUG
  printf("sigchld_action:");
#endif

#if DEBUG
#define CASE(name) case name: printf("si_code = %s(%d)\n", #name, name); break;
  switch(info->si_code) {
    CASE(SI_USER);
    CASE(SI_KERNEL);
    CASE(SI_QUEUE);
    CASE(SI_TIMER);
    CASE(SI_MESGQ);
    CASE(SI_ASYNCIO);
    CASE(SI_SIGIO);
    CASE(SI_TKILL);
    CASE(CLD_EXITED);
    CASE(CLD_KILLED);
    CASE(CLD_DUMPED);
    CASE(CLD_STOPPED);
    CASE(CLD_CONTINUED);
  default:
    printf("si_code = %d\n", info->si_code);
  }  
#endif // DEBUG
  switch(info->si_code) {
  case CLD_STOPPED:
    break;
  default:
    break;
  }
}

void signal_init(void) {
  // setting handler
  struct sigaction sa;
  sa.sa_handler = test_handler;
  sa.sa_flags = 0;
  sigfillset(&sa.sa_mask);
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGQUIT, &sa, NULL);
  /* Ignores SIGTTIN, SIGTTOU */
  sa.sa_handler = SIG_IGN;
  sigaction(SIGTTIN, &sa, NULL);
  sigaction(SIGTTOU, &sa, NULL);
  sa.sa_handler = test_handler;
  sa.sa_flags = SA_SIGINFO;
  sa.sa_sigaction = sigchld_action;
  sigaction(SIGCHLD, &sa, NULL);
  sa.sa_flags = 0;
  sa.sa_handler = sigtstp_handler;
  sigaction(SIGTSTP, &sa, NULL);
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

  signal_init();
  job_init();
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
  while (1) {
    char *line;
    kill_defuncts();
    line = k_getline("ish$ ");
    if (line == NULL) { // error on k_getline
      perror("main.while(1)");
      break;
    }
    if (!strcmp(line, "exit")) {
      break;
    }
    if (! strcmp(line, "bg")) {
      bg_run();
      continue;
    }
    if (! strcmp(line, "fg")) {
      fg_run();
      continue;
    }
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
