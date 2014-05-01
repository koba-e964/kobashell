#include "./kobash.h"

#include <stdlib.h>

void int_list_free(int_list *ptr) {
  int_list *next = ptr->next;
  free(ptr);
  if(next != NULL) {
    int_list_free(next);
  }
}
