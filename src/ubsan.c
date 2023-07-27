#include <stddef.h>
#include <stdint.h>

#include "attributes.h"
#include "kprintf.h"

// https://github.com/llvm/llvm-project/blob/release/16.x/clang/lib/CodeGen/CodeGenFunction.h#L113*/

// EmitCheck
// https://github.com/llvm/llvm-project/blob/8ceba5980c45a7819186bfd699ba3723b9f8b6a0/clang/lib/CodeGen/CodeGenFunction.h#L4674

// EmitCheck source
// https://github.com/llvm/llvm-project/blob/b26dd42458806622e2bd56f6fc62149affe51219/clang/lib/CodeGen/CGExpr.cpp#L3282

// See: compiler-rt/lib/ubsan/ubsan_handlers.h
// See: compiler-rt/lib/ubsan/ubsan_handlers.c

struct source_location {
    const char *filename;
    uint32_t line;
    uint32_t column;
};

struct type_descriptor {
    uint16_t type_kind;
    uint16_t type_info;
    char type_name[1];
};

struct type_mismatch_data {
    struct source_location loc;
    const struct type_descriptor *type;
    unsigned char log_alignment;
    unsigned char type_check_kind;
};

struct alignment_assumption_data {
    struct source_location loc;
    struct source_location assumption_loc;
    const struct type_descriptor *type;
};

struct overflow_data {
    struct source_location loc;
    const struct type_descriptor *type;
};

struct shift_out_of_bounds_data {
    struct source_location loc;
    const struct type_descriptor *lhs_type;
    const struct type_descriptor *rhs_type;
};

struct out_of_bounds_data {
    struct source_location loc;
    const struct type_descriptor *array_type;
    const struct type_descriptor *index_type;
};

struct unreachable_data {
    struct source_location loc;
};

struct pointer_overflow_data {
    struct source_location loc;
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

// TODO: Pointer
void __ubsan_handle_type_mismatch_v1(const struct type_mismatch_data *data,
                                     void *ptr) {
    kprintf("ubsan: type_mismatch_v1 in %s:%u:%u\n", data->loc.filename,
            data->loc.line, data->loc.column);

    uint64_t alignment = 1UL << data->log_alignment;
    if (ptr == NULL) {
        kprintf("%s null pointer of type %s\n",
                type_check_kinds[data->type_check_kind], data->type->type_name);
    } else if ((uint64_t)ptr & (alignment - 1)) {
        kprintf(
            "%s misaligned address %lx for type %s, which requires %lu byte "
            "alignment\n",
            type_check_kinds[data->type_check_kind], (uint64_t)ptr,
            data->type->type_name, alignment);
    } else {
        kprintf(
            "%s address %lx with insufficient space for an object of type %s\n",
            type_check_kinds[data->type_check_kind], (uint64_t)ptr,
            data->type->type_name);
    }
}

// TODO: Pointer, Alignment, Offset
void __ubsan_handle_alignment_assumption(
    __unused const struct alignment_assumption_data *data) {
    kprintf("ubsan: alignment_assumption\n");
}

// TODO: LHS, RHS
void __ubsan_handle_add_overflow(__unused const struct overflow_data *data) {
    kprintf("ubsan: add_overflow\n");
}

// TODO: LHS, RHS
void __ubsan_handle_sub_overflow(__unused const struct overflow_data *data) {
    kprintf("ubsan: sub_overflow\n");
}

// TODO: LHS, RHS
void __ubsan_handle_mul_overflow(__unused const struct overflow_data *data) {
    kprintf("ubsan: mul_overflow\n");
}

// TODO: OldVal
void __ubsan_handle_negate_overflow(__unused const struct overflow_data *data) {
    kprintf("ubsan: negate_overflow\n");
}

// TODO: LHS, RHS
void __ubsan_handle_divrem_overflow(__unused const struct overflow_data *data) {
    kprintf("ubsan: divrem_overflow\n");
}

// TODO: LHS, RHS
void __ubsan_handle_shift_out_of_bounds(
    __unused const struct shift_out_of_bounds_data *data) {
    kprintf("ubsan: shift_out_of_bounds\n");
}

// TODO: Index
void __ubsan_handle_out_of_bounds(
    __unused const struct out_of_bounds_data *data) {
    kprintf("ubsan: out_of_bounds\n");
}

void __ubsan_handle_builtin_unreachable(
    __unused const struct unreachable_data *data) {
    kprintf("ubsan: builtin_unreachable\n");
}

void __ubsan_handle_missing_return(
    __unused const struct unreachable_data *data) {
    kprintf("ubsan: missing_return\n");
}

// TODO: Base, Result
void __ubsan_handle_pointer_overflow(
    __unused const struct pointer_overflow_data *data) {
    kprintf("ubsan: pointer_overflow\n");
}
