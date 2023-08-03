#ifndef AVOCADOS_KASSERT_H_
#define AVOCADOS_KASSERT_H_

#include "panic.h"

#define likely(X) __builtin_expect(!!(X), 1)
#define unlikely(X) __builtin_expect(!!(X), 0)

// TODO: Opt out
// kassert may be opted-out so don't use it if you need to crash the kernel.
#define kassert(EXPR)                                                          \
    {                                                                          \
        if (unlikely(!(EXPR))) {                                               \
            kpanic("%s:%u: Assertion '%s' failed\n", __BASE_FILE__, __LINE__,  \
                   #EXPR);                                                     \
        }                                                                      \
    }

#endif /* ! AVOCADOS_KASSERT_H_ */
