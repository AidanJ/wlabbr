#pragma once
#include <stdlib.h>

void config_init(char const *file_addr);
void config_destroy();

void config_keywords_init();
// Returned value is memory managed
char const *config_keywords_match(char const *buffer, size_t start, size_t end);

