#include "./kobash.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

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
  int result = printf("%s", prompt);
  if (result < 0) {
    perror("k_getline.prompt");
    return NULL;
  }
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
    if (ch == EOF && errno == EINTR) {
      continue; // go on reading even if it is interrupted
    }
    if (ch == EOF && size == 0) {
      free(ptr);
      return NULL;
    }
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
