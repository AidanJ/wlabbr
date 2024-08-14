#include "entries.h"
#include <search.h>
#include <stdlib.h>
#include <string.h>

#define TBL_SIZE 16

// static struct {
//   int table;
// } entries;

void entries_init() {
  if (!hcreate(TBL_SIZE)) {
    // TODO fatal
  }
  ENTRY entry = {.key = "LGTM", .data = (void *) "Looks good to me!"};
  if (!hsearch(entry, ENTER)) {
    // TODO fatal
  }
}

void entries_destroy() { hdestroy(); }

char const *entries_get_matching(char const *buffer, size_t start, size_t end) {
  char *key = malloc(end - start + 1);
  strncpy(key, &buffer[start], end - start);
  key[end] = '\0';

  ENTRY target = {.key = key};

  ENTRY *value = hsearch(target, FIND);
  free(key);

  if (value) {
    return value->data;
  }

  return NULL;
}
