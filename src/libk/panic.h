#ifndef AVOCADOS_PANIC_H_
#define AVOCADOS_PANIC_H_

#include <stdnoreturn.h>

#include "attributes.h"

noreturn void kpanic(const char *fmt, ...) __format(printf, 1, 2);

#endif /* ! AVOCADOS_PANIC_H_ */
