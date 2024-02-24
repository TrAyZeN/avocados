#include <stdbool.h>
#include <stddef.h>

#include "attributes.h"
#include "libk/kassert.h"
#include "libk/kprintf.h"
#include "types.h"

// Based on
// https://github.com/llvm/llvm-project/blob/release/16.x/compiler-rt/lib/ubsan/ubsan_handlers.cpp
// https://github.com/llvm/llvm-project/blob/release/16.x/compiler-rt/lib/ubsan/ubsan_handlers.h
// https://github.com/torvalds/linux/blob/master/lib/ubsan.c

typedef struct {
    const char *filename;
    u32 line;
    u32 column;
} source_location_t;

enum {
    TK_INTEGER = 0x0000,
    TK_FLOAT = 0x0001,
    TK_UNKNOWN = 0xffff,
};

typedef struct {
    u16 type_kind;
    u16 type_info;
    char type_name[1];
} type_descriptor_t;

typedef struct {
    const type_descriptor_t *type;
    void *val;
} value_t;

static inline bool type_is_int(const type_descriptor_t *type) {
    return (type->type_kind == TK_INTEGER);
}

static inline bool type_is_signed_int(const type_descriptor_t *type) {
    return type_is_int(type) && (type->type_info & 1);
}

static inline bool type_is_unsigned_int(const type_descriptor_t *type) {
    return type_is_int(type) && !(type->type_info & 1);
}

static inline unsigned type_integer_bit_width(const type_descriptor_t *type) {
    kassert(type_is_int(type));

    return 1 << (type->type_info >> 1);
}

static bool type_is_inline_int(const type_descriptor_t *type) {
    kassert(type_is_int(type));

    unsigned inline_bits = sizeof(void *) * 8;
    unsigned bits = type_integer_bit_width(type);

    return bits <= inline_bits;
}

typedef i64 s_int_max;
typedef u64 u_int_max;

s_int_max value_get_signed(const value_t *value) {
    kassert(type_is_signed_int(value->type));

    if (type_is_inline_int(value->type)) {
        unsigned extra_bits =
            sizeof(s_int_max) * 8 - type_integer_bit_width(value->type);

        return ((s_int_max)(unsigned long)value->val) << extra_bits
            >> extra_bits;
    }

    if (type_integer_bit_width(value->type) == 64) {
        return *(i64 *)value->val;
    }

    return *(s_int_max *)value->val;
}

u_int_max value_get_unsigned(const value_t *value) {
    kassert(type_is_unsigned_int(value->type));

    if (type_is_inline_int(value->type)) {
        return (u_int_max)value->val;
    }

    if (type_integer_bit_width(value->type) == 64) {
        return *(u64 *)value->val;
    }

    return *(u_int_max *)value->val;
}

static u_int_max value_get_positive_int(const value_t *value) {
    if (type_is_unsigned_int(value->type)) {
        return value_get_unsigned(value);
    }

    s_int_max v = value_get_signed(value);
    kassert(v >= 0);
    return (u_int_max)v;
}

static bool value_is_negative(const value_t *value) {
    return type_is_signed_int(value->type) && value_get_signed(value) < 0;
}

typedef struct {
    source_location_t loc;
    const type_descriptor_t *type;
    unsigned char log_alignment;
    unsigned char type_check_kind;
} type_mismatch_data_t;

typedef struct {
    source_location_t loc;
    source_location_t assumption_loc;
    const type_descriptor_t *type;
} alignment_assumption_data_t;

typedef struct {
    source_location_t loc;
    const type_descriptor_t *type;
} overflow_data_t;

typedef struct {
    source_location_t loc;
    const type_descriptor_t *lhs_type;
    const type_descriptor_t *rhs_type;
} shift_out_of_bounds_data_t;

typedef struct {
    source_location_t loc;
    const type_descriptor_t *array_type;
    const type_descriptor_t *index_type;
} out_of_bounds_data_t;

typedef struct {
    source_location_t loc;
} unreachable_data_t;

typedef struct {
    source_location_t loc;
} pointer_overflow_data_t;

struct nunnull_arg_data {
    source_location_t loc;
    source_location_t attr_loc;
    int arg_index;
};

enum type_check_kind {
    TCK_LOAD,
    TCK_STORE,
    TCK_REFERENCING_BINDING,
    TCK_MEMBER_ACCESS,
    TCK_MEMBER_CALL,
    TCK_CONSTRUCTOR_CALL,
    TCK_DOWNCAST_POINTER,
    TCK_DOWNCAST_REFERENCE,
    TCK_UPCAST,
    TCK_UPCAST_TO_VIRTUAL_BASE,
    TCK_NONNULL_ASSIGN,
    TCK_DYNAMIC_OPERATION
};

const char *const type_check_kinds[] = { "load of",
                                         "store to",
                                         "reference binding to",
                                         "member access within",
                                         "member call on",
                                         "constructor call on",
                                         "downcast of",
                                         "downcast of",
                                         "upcast of",
                                         "cast to virtual base of",
                                         "_Nonnull binding to",
                                         "dynamic operation on" };

#define PRINT_UB_LOCATION(UB_NAME, LOC)                                        \
    kprintf("ubsan: %s in %s:%u:%u\n", (UB_NAME), (LOC).filename, (LOC).line,  \
            (LOC).column)

void __ubsan_handle_type_mismatch_v1(const type_mismatch_data_t *data,
                                     void *ptr) {
    PRINT_UB_LOCATION("type_mismatch_v1", data->loc);

    u64 alignment = 1UL << data->log_alignment;
    if (ptr == NULL) {
        kprintf("%s null pointer of type %s\n",
                type_check_kinds[data->type_check_kind], data->type->type_name);
    } else if ((u64)ptr & (alignment - 1)) {
        kprintf("%s misaligned address %p for type %s, which requires %lu byte "
                "alignment\n",
                type_check_kinds[data->type_check_kind], ptr,
                data->type->type_name, alignment);
    } else {
        kprintf(
            "%s address %p with insufficient space for an object of type %s\n",
            type_check_kinds[data->type_check_kind], ptr,
            data->type->type_name);
    }
}

// TODO: Pointer, Alignment, Offset
void __ubsan_handle_alignment_assumption(
    __unused const alignment_assumption_data_t *data) {
    kprintf("ubsan: alignment_assumption\n");
}

static void handle_integer_overflow_impl(const overflow_data_t *data, void *lhs,
                                         const char *operator, void * rhs) {
    if (type_is_signed_int(data->type)) {
        kprintf("signed integer overflow: %ld %s %ld cannot be represented in "
                "type %s\n",
                (i64)lhs, operator,(i64) rhs, data->type->type_name);
    } else {
        kprintf("unsigned integer overflow: %lu %s %lu cannot be represented "
                "in type %s\n",
                (u64)lhs, operator,(u64) rhs, data->type->type_name);
    }
}

void __ubsan_handle_add_overflow(const overflow_data_t *data, void *lhs,
                                 void *rhs) {
    PRINT_UB_LOCATION("add_overflow", data->loc);

    handle_integer_overflow_impl(data, lhs, "+", rhs);
}

void __ubsan_handle_sub_overflow(const overflow_data_t *data, void *lhs,
                                 void *rhs) {
    PRINT_UB_LOCATION("sub_overflow", data->loc);

    handle_integer_overflow_impl(data, lhs, "-", rhs);
}

void __ubsan_handle_mul_overflow(const overflow_data_t *data, void *lhs,
                                 void *rhs) {
    PRINT_UB_LOCATION("mul_overflow", data->loc);

    handle_integer_overflow_impl(data, lhs, "*", rhs);
}

void __ubsan_handle_negate_overflow(const overflow_data_t *data,
                                    void *old_val) {
    PRINT_UB_LOCATION("negate_overflow", data->loc);

    if (type_is_signed_int(data->type)) {
        kprintf("negation of %ld cannot be represented in type %s; cast to an "
                "unsigned type to negate this value to itself\n",
                (i64)old_val, data->type->type_name);
    } else {
        kprintf("negation of %lu cannot be represented in type %s\n",
                (u64)old_val, data->type->type_name);
    }
}

void __ubsan_handle_divrem_overflow(const overflow_data_t *data, void *lhs,
                                    void *rhs) {
    PRINT_UB_LOCATION("divrem_overflow", data->loc);

    value_t rhs_val = { .type = data->type, .val = rhs };

    if (type_is_signed_int(data->type) && value_get_signed(&rhs_val) == -1) {
        kprintf("division of %ld by -1 cannot be represented in type %s\n",
                (i64)lhs, data->type->type_name);
    } else {
        kprintf("division by zero\n");
    }
}

void __ubsan_handle_shift_out_of_bounds(const shift_out_of_bounds_data_t *data,
                                        void *lhs, void *rhs) {
    PRINT_UB_LOCATION("shift_out_of_bounds", data->loc);

    value_t lhs_val = { .type = data->lhs_type, .val = lhs };
    value_t rhs_val = { .type = data->rhs_type, .val = rhs };

    if (value_is_negative(&rhs_val)
        || value_get_positive_int(&rhs_val)
            >= type_integer_bit_width(data->lhs_type)) {
        if (value_is_negative(&rhs_val)) {
            kprintf("shift exponent %ld is negative\n", (i64)rhs);
        } else {
            kprintf("shift exponent %lu is too large for %u-bit type %s\n",
                    (u64)rhs, type_integer_bit_width(data->lhs_type),
                    data->lhs_type->type_name);
        }
    } else {
        if (value_is_negative(&lhs_val)) {
            kprintf("left shift of negative value %ld\n", (i64)lhs);
        } else {
            kprintf("left shift of %lu by %lu places cannot be represented in "
                    "type %s\n",
                    (u64)lhs, (u64)rhs, data->lhs_type->type_name);
        }
    }
}

void __ubsan_handle_out_of_bounds(const out_of_bounds_data_t *data,
                                  void *index) {
    PRINT_UB_LOCATION("out_of_bounds", data->loc);

    kprintf("index %lu out of bounds for type %s\n", (u64)index,
            data->array_type->type_name);
}

void __ubsan_handle_builtin_unreachable(
    __unused const unreachable_data_t *data) {
    kprintf("ubsan: builtin_unreachable\n");
}

void __ubsan_handle_missing_return(__unused const unreachable_data_t *data) {
    kprintf("ubsan: missing_return\n");
}

void __ubsan_handle_pointer_overflow(const pointer_overflow_data_t *data,
                                     void *base, void *result) {
    PRINT_UB_LOCATION("pointer_overflow", data->loc);

    if (base == NULL && result == NULL) {
        kprintf("applying zero offset to null pointer\n");
    } else if (base == NULL && result != NULL) {
        kprintf("applying non-zero offset %p to null pointer\n", result);
    } else if (base != NULL && result == NULL) {
        kprintf("applying non-zero offset to non-null pointer %p "
                "produced null "
                "pointer\n",
                base);
    } else if (((i64)base >= 0) == ((i64)result >= 0)) {
        if (base > result) {
            kprintf("addition of unsigned offset to %p overflowed to %p\n",
                    base, result);
        } else {
            kprintf("subtraction of unsigned offset from %p overflowed to %p\n",
                    base, result);
        }
    } else {
        kprintf("pointer index expression with base %p overflowed to "
                "%p\n",
                base, result);
    }
}

void __ubsan_handle_nonnull_arg(__unused const pointer_overflow_data_t *data) {
    kprintf("ubsan: nunnull_arg\n");
}
