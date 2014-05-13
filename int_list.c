#include "./kobash.h"

#include <stdlib.h>

int_list *int_list_new(void) {
  return (int_list *) malloc(sizeof(int_list));
}

void int_list_free(int_list *ptr) {
  int_list *next = ptr->next;
  free(ptr);
  if(next != NULL) {
    int_list_free(next);
  }
}
