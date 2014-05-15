#include "./kobash.h"

#include <stdio.h>
#include <stdlib.h>

int_list *int_list_new(void) {
  return (int_list *) malloc(sizeof(int_list));
}

void int_list_add(int_list *ls, int value) {
  int_list *cur = ls;
  while (cur->next) {
    cur = cur->next;
  }
  cur->next = int_list_new();
  cur->val = value;
}

void int_list_print(int_list *ls) {
  int_list *cur = ls;
  printf("[");
  while (cur->next) {
    printf("%d", cur->val);
    cur = cur->next;
    if (cur->next) {
      printf(", ");
    }
  }
  printf("]");
}

int int_list_remove(int_list *ls, int value) {
  int_list *cur = ls;
  while (cur->next) {
    if (cur->val == value) {
      // remove
      int_list *next = cur->next;
      cur->val = next->val;
      cur->next = next->next;
      return 1;
    }
  }
  return 0;
}

void int_list_free(int_list *ptr) {
  int_list *next = ptr->next;
  free(ptr);
  if(next != NULL) {
    int_list_free(next);
  }
}
