#ifndef KOBASH_H_aa406f7d
#define KOBASH_H_aa406f7d

#include "config.h"

/* getline.c */
char *k_getline(char *prompt);

/* int_list.c */
typedef struct int_list_ {
  int val;                // if p->next == 0, val has no meanings.
  struct int_list_ *next; // if p->next == 0, p stands for [].
} int_list;

void int_list_free(int_list *ptr);


#endif
