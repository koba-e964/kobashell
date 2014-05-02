#include "./kobash.h"

#include <string.h>
#include <unistd.h>
#include "parser/parse.h"
#include <stdio.h>
#include <stdlib.h>


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
  while (1) {
    char *line;
    kill_defuncts();
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
