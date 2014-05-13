#include "./kobash.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>




#if DEBUG
int CLOSE(int fd) {
  printf("closing %d\n", fd);
  return close(fd);
}
#else
#define CLOSE close
#endif

void signal_init(void);
extern int wait_cont;


void child(char *const *envp, int pre_fd, int pipefd[2], process *plist, int group_id) {
  int result;
  char *exec_name; /* the name of the executable */
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
  if (group_id == -2) { // This process is the first process.
    group_id = getpid();
  }
  if (setpgid(0, group_id) < 0) {
    perror("child.setpgid");
    fprintf(stderr, "[pid = %d ]\n", getpid());
  }
  exec_name = search_path(plist->program_name);
  if (exec_name == NULL) {
    printf("ish: not found: %s\n", plist->program_name);
    exit(EXIT_FAILURE);
  }
  signal_init(); // set signals
  result = execve(exec_name, plist->argument_list, envp);
  printf("Error: status = %d\n", result);
  perror("ish");
  fprintf(stderr, "  in attempt to execute \"%s\"\n", plist->program_name);
  exit(result);
}


/*
  Executes a job.
*/
void execute_job(job* job,char *const envp[]) {
  pid_t pid;
  pid_t group_id = -2; // the process group of children, -2 if not initialized
  int pipefd[2];
  int status;
  int pre_fd = -2; // fd of previous process' stdout. If -2, it is not assigned.
  process *plist = job->process_list;
  int_list *pids, *pid_cur;

  wait_cont = 1; // continue waiting if FOREGROUND
  pids = malloc(sizeof(int_list));
  pids->val = -1;
  pids->next = NULL;
  pid_cur = pids;
  
  for (; plist; plist = plist->next) {
    if (plist->next) {
      int result = pipe(pipefd);
#if DEBUG
      printf("pipefd: (%d, %d)\n", pipefd[0], pipefd[1]);
#endif
      if (result == -1) {
        perror("execute_job.for.if.pipe");
        return;
      }
    }
    pid = fork();
    if (pid == 0) {
      child(envp, pre_fd, pipefd, plist, group_id);
      assert(!"unreachable");
    }
// parent process
    if (pre_fd != -2) {
      CLOSE(pre_fd);
    }
    if (plist->next) {
      CLOSE(pipefd[1]);
      pre_fd = pipefd[0]; //the read-end of the pipe
    }
    if (group_id == -2) {
#if DEBUG
      fprintf(stderr, "group_id = %d\n", pid);
#endif
      group_id = pid; // The first process' pid is the group id.
      if (job->mode == FOREGROUND) {
        if (tcsetpgrp(0, group_id) < 0) {
          perror("shell.tcsetpgrp(child)");
        }
      }
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
    while (pid_cur->next && wait_cont) {
#if DEBUG
      fprintf(stderr, "waiting %d...:\n ", pid_cur->val); 
#endif
      int result = waitpid(pid_cur->val, &status, 0);
      if (result < 0) {
        if (errno == EINTR) {
          // continue waiting
#if DEBUG
          printf("cw\n");
#endif
          continue;
        }
        perror("FOREGROUND.waitpid");
	pid = pid_cur->val;
        fprintf(stderr, "pid = %d\n", pid);
        pid_cur = pid_cur->next;
        continue;
      }
#if DEBUG
      printf("[%d] finished (status = %d)\n", pid_cur->val, status);
#endif
      pid_cur = pid_cur->next;
    }
    int_list_free(pids);
    if (tcsetpgrp(0, getpid()) < 0) { // set shell to be foreground
      perror("execute_job.FOREGROUND.tcsetpgrp");
    }
    break;
  case BACKGROUND:
    pid_cur = pids;
    printf("[created] (pid = ");
    while (pid_cur->next) {
      printf("%d", pid_cur->val);
      pid_cur = pid_cur->next;
      if(pid_cur->next) {
        printf(", ");
      }
    }
    printf(")\n");
    int_list_free(pids);
    break;
  default:
    assert(!"not reachable");
  }
}

void kill_defuncts(void) {
  pid_t pid;
  int status;
  while (1) {
    pid = waitpid(-1, &status, WNOHANG);
    if (pid == -1) {
      if (errno == ECHILD) { /* There are no child processes. */
	break;
      }
      perror("kill_defuncts");
      continue;
    }
    if (pid == 0) { /* No child processes are terminated. */
      break;
    }
    printf("[terminated] pid = %d, status = %d\n", pid, status);
  }
}
