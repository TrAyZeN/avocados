#ifndef AVOCADOS_KASSERT_H_
#define AVOCADOS_KASSERT_H_

#include "panic.h"

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

// TODO: Opt out
// kassert may be opted-out so don't use it if you need to crash the kernel.
#define kassert(expr)                                                          \
    {                                                                          \
        if (unlikely(!(expr))) {                                               \
            kpanic("%s:%u: Assertion '%s' failed\n", __BASE_FILE__, __LINE__,  \
                   #expr);                                                     \
        }                                                                      \
    }

#endif /* ! AVOCADOS_KASSERT_H_ */
