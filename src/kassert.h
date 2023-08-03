#ifndef AVOCADOS_KASSERT_H_
#define AVOCADOS_KASSERT_H_

#include <stdbool.h>

#include "panic.h"

#define likely(X) __builtin_expect(!!(X), 1)
#define unlikely(X) __builtin_expect(!!(X), 0)

// TODO: Opt out
// kassert may be opted-out so don't use it if you need to crash the kernel.
#define kassert(EXPR)                                                          \
    do {                                                                       \
        if (unlikely(!(EXPR))) {                                               \
            kpanic("%s:%u: Assertion '%s' failed\n", __BASE_FILE__, __LINE__,  \
                   #EXPR);                                                     \
        }                                                                      \
    } while (false)

// Casts are needed as _Generic expressions must be valid under all
// circumstances. (See https://stackoverflow.com/a/24746034)
#define kassert_eq(EXPR1, EXPR2)                                               \
    do {                                                                       \
        typeof(EXPR1) _expr1 = (EXPR1);                                        \
        typeof(EXPR2) _expr2 = (EXPR2);                                        \
        if (unlikely(_expr1 != _expr2)) {                                      \
            _Generic((EXPR1), \
                char : _report_kassert_eq_failed( \
                    "%c", #EXPR1, #EXPR2, (char)_expr1, (char)_expr2), \
                unsigned char : _report_kassert_eq_failed( \
                    "%u", #EXPR1, #EXPR2, (unsigned char)_expr1, (unsigned char)_expr2), \
                int: _report_kassert_eq_failed( \
                    "%d", #EXPR1, #EXPR2, (int)_expr1, (int)_expr2), \
                unsigned int: _report_kassert_eq_failed( \
                    "%u", #EXPR1, #EXPR2, (unsigned int)_expr1, (unsigned int)_expr2), \
                long: _report_kassert_eq_failed( \
                    "%ld", #EXPR1, #EXPR2, (long)_expr1, (long)_expr2), \
                unsigned long: _report_kassert_eq_failed( \
                    "%lu", #EXPR1, #EXPR2, (unsigned long)_expr1, (unsigned long)_expr2), \
                long long: _report_kassert_eq_failed( \
                    "%lld", #EXPR1, #EXPR2, (long long)_expr1, (long long)_expr2), \
                unsigned long long: _report_kassert_eq_failed( \
                    "%llu", #EXPR1, #EXPR2, (unsigned long long)_expr1, (unsigned long long)_expr2), \
                char *: _report_kassert_eq_failed( \
                    "%s", #EXPR1, #EXPR2, (char *)(unsigned long)_expr1, (char *)(unsigned long)_expr2), \
                void *: _report_kassert_eq_failed( \
                    "%lx", #EXPR1, #EXPR2, (unsigned long)_expr1, (unsigned long)_expr2), \
                default: _report_kassert_eq_failed( \
                     "%lx", #EXPR1, #EXPR2, (unsigned long)_expr1, (unsigned long)_expr2));                                                \
        }                                                                      \
    } while (false)

#define _report_kassert_eq_failed(FMT, EXPR1_STR, EXPR2_STR, EXPR1_VAL,        \
                                  EXPR2_VAL)                                   \
    kpanic("%s:%u: Assertion '%s == %s' failed\n"                              \
           "  %s = " FMT "\n"                                                  \
           "  %s = " FMT "\n",                                                 \
           __BASE_FILE__, __LINE__, EXPR1_STR, EXPR2_STR, EXPR1_STR,           \
           EXPR1_VAL, EXPR2_STR, EXPR2_VAL)

#endif /* ! AVOCADOS_KASSERT_H_ */
