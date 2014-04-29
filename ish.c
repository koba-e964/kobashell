#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#include <assert.h>
#include <string.h>
#include <unistd.h>
#include "parser/parse.h"
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#define DEBUG 0


void print_job_list(job*);

typedef struct int_list_ {
  int val;                // if p->next == 0, val has no meanings.
  struct int_list_ *next; // if p->next == 0, p stands for [].
} int_list;

void int_list_free(int_list *ptr) {
  int_list *next = ptr->next;
  free(ptr);
  if(next != NULL) {
    int_list_free(next);
  }
}

/*
  Executes a job.
  cleanup: NO file descriptors are closed.
*/
void execute_job(job* job,char *const envp[]) {
  pid_t pid;
  int pipefd[2];
  int status;
  int pre_fd = -2; // fd of previous process' stdout. If -2, it is not assigned.
  process *plist = job->process_list;
  int_list *pids, *pid_cur;
  pids = malloc(sizeof(int_list));
  pids->val = -1;
  pids->next = NULL;
  pid_cur = pids;
  
  for (; plist; plist = plist->next) {
    if (plist->next) {
      int result = pipe(pipefd);
      if (result == -1) {
        perror("execute_job.for.if.pipe");
        return;
      }
    }
    pid = fork();
    if (pid == 0) {
      int result;
      if (pre_fd != -2) {
        dup2(pre_fd, 0);
      }
      if (plist->input_redirection) { // stdin is redirected.
        int fd = open(plist->input_redirection, O_RDONLY);
        if (fd == -1) {
          fprintf(stderr, "error in opening %s (input)\n", plist->input_redirection);
          exit(1);
        }
        dup2(fd, 0); // stdin was redirected.
      }
      if (plist->next) {
        dup2(pipefd[1], 1);
      }
      if (plist->output_redirection) { //stdout is redirected.
        int fd = open(plist->output_redirection, O_WRONLY | O_CREAT | (plist->output_option == APPEND ? O_APPEND : 0), 0777);
        if (fd == -1) {
          fprintf(stderr, "error in opening %s (output)\n", plist->output_redirection);
          exit(1);
        }
        dup2(fd, 1); // stdout was redirected.
      }
      result = execve(plist->program_name, plist->argument_list, envp);
#if DEBUG
      printf("Error: status = %d\n", result);
#endif
      perror("ish");
      fprintf(stderr, "  in attempt to execute \"%s\"\n", plist->program_name);
      exit(result);
    }
    if (pre_fd != -2) {
      close(pre_fd);
    }
    if (plist->next) {
      close(pipefd[1]);
      pre_fd = pipefd[0]; //the read-end of the pipe
    }
    pid_cur->val = pid;
    pid_cur->next = malloc(sizeof(int_list));
    pid_cur = pid_cur->next;
    pid_cur->val = -1;
    pid_cur->next = NULL;
  }
  switch (job->mode) {
  case FOREGROUND:
    pid_cur = pids;
    while (pid_cur->next) {
      waitpid(pid_cur->val, &status, 0);
#if DEBUG
      printf("[%d] finished (status = %d)\n", pid_cur->val, status);
#endif
      pid_cur = pid_cur->next;
    }
    int_list_free(pids);
    break;
  case BACKGROUND:
#if DEBUG
    pid_cur = pids;
    printf("[pid = ");
    while (pid_cur->next) {
      printf("%d", pid_cur->val);
      pid_cur = pid_cur->next;
      if(pid_cur->next) {
        printf(", ");
      }
    }
    printf("]\n (created)");
#endif
    int_list_free(pids);
    break;
  default:
    assert(!"not reachable");
  }
}


int main(int argc, char *const argv[], char *const envp[]) {
  char s[LINELEN];
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

  while(get_line(s, LINELEN)) {

    if(!strcmp(s, "exit\n"))
      break;
    curr_job = parse_line(s);
    while (curr_job != NULL) {
      execute_job(curr_job, envp);
      curr_job = curr_job->next;
    }
    free_job(curr_job);
  }

  return 0;
}

