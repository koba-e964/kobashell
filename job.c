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


/*
  list of all living process groups
*/
static int_list *pgroups; /* Suspended process groups  */
static int_list *bgpg;    /* Background process groups */

void job_init(void) {
  pgroups = int_list_new();
  bgpg = int_list_new();
}

static void pgroups_add(pid_t pgid) {
  int_list_add(pgroups, pgid);
}

#if DEBUG
static void print_pgroups(void) {
  printf("pgroups: ");
  int_list_print(pgroups);
  puts("");
}
#endif

static int pgroups_remove(pid_t pgid) {
  return int_list_remove(pgroups, pgid);
}

static void bgpg_add(pid_t pgid) {
  int_list_add(bgpg, pgid);
}

#if DEBUG
static void print_bgpg(void) {
  printf("bgpg: ");
  int_list_print(bgpg);
  puts("");
}
#endif

static int bgpg_remove(pid_t pgid) {
  return int_list_remove(bgpg, pgid);
}

static void pg_wait(pid_t pgid);


static void child(char *const *envp, int pre_fd, int pipefd[2], process *plist, int group_id) {
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
    int first = group_id == -2;
    if (first) {
#if DEBUG
      fprintf(stderr, "group_id = %d\n", pid);
#endif
      group_id = pid; // The first process' pid is the group id.
    }
    if (setpgid(pid, group_id) < 0) {
      perror("child.setpgid");
      fprintf(stderr, "[pid = %d ]\n", getpid());
    }
    if (first && job->mode == FOREGROUND) {
      if (tcsetpgrp(0, group_id) < 0) {
        perror("shell.tcsetpgrp(child)");
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
    pg_wait(group_id);
    int_list_free(pids);
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
    bgpg_add(group_id);
    int_list_free(pids);
    break;
  default:
    assert(!"not reachable");
  }
#if DEBUG
  print_pgroups();
  print_bgpg();
#endif
}

void bg_run(void) {
  pid_t pgid;
  if (pgroups->next == NULL) {
    puts("There are no suspended jobs.");
    return;
  }
  pgid = pgroups->val;
  pgroups_remove(pgid);
#if DEBUG
    fprintf(stderr, "running pg %d...\n", pgid);
#endif
  if (kill(-pgid, SIGCONT) < 0) {
    perror("bg_run.kill");
  }
  bgpg_add(pgid);
}

/*
 This makes background process groups run in f
*/
void fg_run(void) {
  pid_t pgid;
  if (bgpg->next == NULL) {
    puts("There are no background jobs.");
    return;
  }
  pgid = bgpg->val;
  bgpg_remove(pgid);
#if DEBUG
  fprintf(stderr, "moving pg %d to foreground...\n", pgid);
#endif
  pg_wait(pgid);
}

/*
  Waits for process group of id pgid.
  This is blocking.
*/
static void pg_wait(pid_t pgid) {
  if (tcsetpgrp(0, pgid) < 0) {
    perror("pg_wait.tcsetpgrp(child)");
  }
  while (1) {
    int status;
    int result = waitpid(-pgid, &status, WUNTRACED);
    if (result < 0) {
      if (errno == EINTR) {
        // continue waiting
#if DEBUG
        printf("pg_wait.cw\n");
#endif
        continue;
      }
      if (errno == ECHILD) { // no unwaited-for children
        break;
      }
      perror("pg_wait.waitpid");
      fprintf(stderr, "(pgid = %d)\n", pgid);
      break;
    }
#if DEBUG
    printf("pg_wait: pid = %d\n", result);
#endif
    if (WIFSTOPPED(status)) {
      printf("STOPPED: pid = %d, groupid= %d\n", result, pgid);
      pgroups_add(pgid);
      break;
    }
  }
  if (tcsetpgrp(0, getpid()) < 0) { // set shell to be foreground
    perror("pg_wait.tcsetpgrp(shell)");
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
    /* If bgpg does not contain pid, this will do nothing. */
    bgpg_remove(pid);
  }
}
