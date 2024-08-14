#pragma once
#include <stdlib.h>

void entries_init();
void entries_destroy();
char const* entries_get_matching(char const* buffer, size_t start, size_t end);
