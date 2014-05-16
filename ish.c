#include "./kobash.h"

#include <assert.h>
#include <string.h>
#include <unistd.h>
#include "parser/parse.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *const argv[], char *const envp[]) {
  job *curr_job;
  char *line;

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
  for (line = NULL; 1; free(line)) {
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
  }
  return 0;
}
