#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "parser/parse.h"
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include "./kobash.h"

#if DEBUG
int CLOSE(int fd) {
  printf("closing %d\n", fd);
  return close(fd);
}
#else
#define CLOSE close
#endif

char *path = NULL;
int_list *tok_path = NULL;

/*
  Initalizes internal data for search_path.
  We should release
   * path (free)
  after use.
*/
void init_path(char *const envp[]) {
  //search for path
  {
    char * const *cur_env = envp;
    for(; !path && *cur_env; ++cur_env) {
      if (strncmp(*cur_env, "PATH=", 5) == 0) {
        path = strdup((*cur_env) + 5); /* "PATH=" */
      }
    }
#if DEBUG
    printf("$PATH = %s\n", path);
#endif
  }
  int_list* pts = (int_list *) malloc(sizeof(int_list));
  pts->val = 0;
  pts->next = NULL;
  int_list* cur = pts;
  int pos = 0;
  int last = 0;
  for (; 1; ++pos) {
    char pp = path[pos];
    if (pp == ':' || pp == '\0') {
      if (last < pos) {
        cur->val = last;
        cur->next = (int_list *) malloc(sizeof(int_list));
        cur = cur->next;
      }
      path[pos] = '\0';
      last = pos + 1;
    }
    if (pp == '\0') {
      break;
    }
  }
  tok_path = pts;
#if DEBUG
  printf("tokenized path:[");
  for (cur = tok_path; cur->next != NULL; cur = cur->next) {
    printf("%s\n", path + cur->val);
  }
  printf("]\n");
#endif
}

/*
  If filename contains no '/'s, it searches $PATH for an executable whose name is filename.
  Otherwise, it will search for current directory only.
  If there are no such files, this will return NULL.
  The returned string has to be released by free unless it is NULL.
*/
char *search_path(char const *filename) {
  int_list *cur;
  int flen = strlen(filename);
  if (strstr(filename, "/")) { /* filename contains a '/'. */
    int result = access(filename, F_OK);
    return result == -1 ? NULL : strdup(filename);
  }
  for(cur = tok_path; cur->next != NULL; cur = cur->next) {
    int slen = strlen(path + cur->val);
    char *buf = malloc(slen + flen + 2);
    /* creates (an element of PATH) + "/" + filename */
    strncpy(buf, path + cur->val, slen);
    buf[slen] = '/';
    strncpy(buf + slen + 1,filename, flen);
    buf[slen + 1 + flen] = '\0';
#if DEBUG
    printf("Checking %s...\n", buf);
#endif
    int result = access(buf, F_OK);
    if (result == -1) { /* the file doesn't exist */
      errno = 0;
    } else { /* found! */
      return buf;
    }
    free(buf);
  }
  return NULL;
}


/*
  Executes a job.
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
      exec_name = search_path(plist->program_name);
      if (exec_name == NULL) {
	printf("ish: not found: %s\n", plist->program_name);
	exit(EXIT_FAILURE);
      }
      result = execve(exec_name, plist->argument_list, envp);
#if DEBUG
      printf("Error: status = %d\n", result);
#endif
      perror("ish");
      fprintf(stderr, "  in attempt to execute \"%s\"\n", plist->program_name);
      exit(result);
    }
    if (pre_fd != -2) {
      CLOSE(pre_fd);
    }
    if (plist->next) {
      CLOSE(pipefd[1]);
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
    printf("] (created)\n");
#endif
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
#if DEBUG
    if (pid != 0) {
      printf("nohang finish: pid = %d, status = %d\n", pid, status);
    }
#endif
    if (pid == 0) {
      break;
    }
    if (pid == -1 && errno == ECHILD) {
      break;
    }
  }
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
  init_path(envp); /* initializes path, tok_path */
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
