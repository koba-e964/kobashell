#include "./kobash.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static char *path = NULL;
static int_list *tok_path = NULL;

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

