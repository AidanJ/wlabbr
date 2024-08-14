#include "log.h"
#include <stdarg.h>
#include <stdio.h>

static enum log_level min_level = WARNING;

void log_init(enum log_level level) { min_level = level; }

void log_report(enum log_level level, char const *format, ...) {
  if(level < min_level) {
    return;
  }

  va_list args;
  va_start(args, format);
  fprintf(
      stderr, "[%s] ",
      level == ERROR     ? "ERROR"
      : level == WARNING ? "WARN"
                         : "INFO"
  );
  vfprintf(stderr, format, args);
  fprintf(stderr, "\n");
  va_end(args);
}
