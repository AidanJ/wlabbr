#pragma once
#include <stdint.h>

enum log_level : uint8_t {
  INFO = 0, WARNING, ERROR
};

void log_init(enum log_level level);

void log_report(enum log_level level, char const* format, ...);
