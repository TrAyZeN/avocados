#include "log.h"

#include <stdarg.h>

#include "kprintf.h"

static const char *level_str[] = {
    [LOG_LEVEL_NONE] = "NONE",   [LOG_LEVEL_ERROR] = "ERROR",
    [LOG_LEVEL_WARN] = "WARN",   [LOG_LEVEL_INFO] = "INFO",
    [LOG_LEVEL_DEBUG] = "DEBUG",
};

// TODO: log only certain level
void log(enum log_level level, const char *fmt, ...) {
    kprintf("[%s] ", level_str[level]);

    va_list args;
    va_start(args, fmt);
    kvprintf(fmt, args);
    va_end(args);
}
