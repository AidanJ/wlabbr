#include "config.h"
#include "log.h"

#include <search.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cjson/cJSON.h>

static cJSON *head = NULL;

// NOTE: buffer has to be memory managed
static char const *config_read_file(char const *file_addr) {
  FILE *file = fopen(file_addr, "r");
  if (!file) {
    log_report(ERROR, "could not read config file");
    exit(EXIT_FAILURE);
  }

  fseek(file, 0, SEEK_END);
  size_t file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  char *buffer = malloc(file_size + 1);
  if (fread(buffer, sizeof(char), file_size, file) != file_size) {
    log_report(ERROR, "failed while reading config file");
    exit(EXIT_FAILURE);
  }
  buffer[file_size] = '\0';

  fclose(file);

  return buffer;
}

void config_init(char const *file_addr) {
  head = cJSON_Parse(config_read_file(file_addr));
  if (!head) {
    log_report(
        ERROR, "could not parse json config file: %s", cJSON_GetErrorPtr()
    );
    exit(EXIT_FAILURE);
  }
  free(head);
}

void config_destroy() {
  hdestroy();
  cJSON_Delete(head);
}

void config_keywords_init() {
  cJSON *keywords = cJSON_GetObjectItemCaseSensitive(head, "keywords");
  if (!keywords) {
    log_report(ERROR, "\"keywords\" object is missing");
    exit(EXIT_FAILURE);
  }

  int const entries_count = cJSON_GetArraySize(keywords);
  // NOTE: hcreate may adjust count upwards to improve load factor but maybe
  // also apply it here?
  if (!hcreate(entries_count)) {
    log_report(ERROR, "failed to initialize keywords table");
    exit(EXIT_FAILURE);
  }

  cJSON *abbr = NULL;
  ENTRY entry = {0};
  // TODO: check for bad key values (e.g. keys consisting of whitespaces)
  cJSON_ArrayForEach(abbr, keywords) {
    // NOTE: fields are owned by cJSON tree therefore they should not be
    // deallocated while `config_keywords_match` is still being used
    entry.key = abbr->string;
    entry.data = abbr->valuestring;
    if (!entry.key || !entry.data) {
      log_report(ERROR, "skipping keywords enty, invalid field(s)");
    } else {
      if (!hsearch(entry, ENTER)) {
        log_report(
            ERROR, "could not insert abbreviation (%s) to keywords table",
            entry.key
        );
      }
      log_report(INFO, "scanned key: %s, value: %s", abbr->string, abbr->valuestring);
    }
  }
}

char const *config_keywords_match(
    char const *const buffer, size_t const start, size_t const end
) {
  size_t const key_str_size = end - start + 1;
  char *key = malloc(key_str_size + 1);
  strncpy(key, &buffer[start], key_str_size);
  key[key_str_size] = '\0';

  ENTRY target = {.key = key};

  ENTRY *value = hsearch(target, FIND);
  free(key);

  if (value) {
    return value->data;
  }

  return NULL;
}
