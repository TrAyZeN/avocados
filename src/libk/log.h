#ifndef AVOCADOS_LOG_H_
#define AVOCADOS_LOG_H_

#include "attributes.h"

enum log_level {
    LOG_LEVEL_NONE = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARN = 2,
    LOG_LEVEL_INFO = 3,
    LOG_LEVEL_DEBUG = 4,
    LOG_LEVEL_ALL = 5,
};

void log(enum log_level, const char *fmt, ...) __format(printf, 2, 3);

#endif /* ! AVOCADOS_LOG_H_ */
