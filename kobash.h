#ifndef KOBASH_H_aa406f7d
#define KOBASH_H_aa406f7d

#include "config.h"
#include "parser/parse.h"

/* getline.c */
char *k_getline(char *prompt);

/* int_list.c */
typedef struct int_list_ {
  int val;                // if p->next == 0, val has no meanings.
  struct int_list_ *next; // if p->next == 0, p stands for [].
} int_list;

void int_list_free(int_list *ptr);

/* job.c */

/*
  Executes a job.
*/
void execute_job(job* job,char *const envp[]);

/*
  Kills all <defunct> processes.
*/
void kill_defuncts(void);

/* path.c */

/*
  Initalizes internal data for search_path.
*/
void init_path(char *const envp[]);

/*
  If filename contains no '/'s, it searches $PATH for an executable whose name is filename.
  Otherwise, it will search for current directory only.
  If there are no such files, this will return NULL.
  The returned string has to be released by free unless it is NULL.
*/
char *search_path(char const *filename);

#endif
