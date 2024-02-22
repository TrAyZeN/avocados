#ifndef AVOCADOS_KPRINTF_H_
#define AVOCADOS_KPRINTF_H_

#include <stdarg.h>

#include "attributes.h"

void puts(const char *str);
void putchar(char c);

void kprintf(const char *fmt, ...) __format(printf, 1, 2);
void kvprintf(const char *fmt, va_list ap) __format(printf, 1, 0);

#endif /* ! AVOCADOS_KPRINTF_H_ */
