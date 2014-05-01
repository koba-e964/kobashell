#include "./kobash.h"

#include <stdio.h>
#include <stdlib.h>

#if USE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

char *k_getline(char *prompt) {
#if USE_READLINE
  char *line = readline(prompt);
  add_history(line);
  return line;
#else /* USE_READLINE */
  int capacity = 32;
  int size = 0;
  char *ptr;
  printf("%s", prompt);
  ptr = malloc(capacity);
  
  while (1) {
    if (size >= capacity) { //expands buffer
      void *nptr;
      capacity += 32;
      nptr = realloc(ptr, capacity);
      if(nptr == NULL) { // allocation failed
        perror("getline.getline");
        free(ptr);
        return NULL;
      }
      ptr = nptr;
    }
    int ch = getchar();
    if (ch == EOF || ch == '\n') {
      ptr[size] = '\0';
      break;
    }
    ptr[size] = ch;
    ++size;
  }
  return ptr;
#endif
} /* char *getline(char *prompt) */
