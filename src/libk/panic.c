#include <stdarg.h>

#include "arch/instr.h"
#include "kprintf.h"
#include "panic.h"

// TODO: Mark kpanic function as cold

// Report a fatal error and hang...
noreturn void kpanic(const char *fmt, ...) {
    cli();

    va_list ap;
    va_start(ap, fmt);
    kvprintf(fmt, ap);
    va_end(ap);

    while (1) {
        hlt();
    }
}
