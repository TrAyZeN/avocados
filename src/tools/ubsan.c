#include <stddef.h>

#include "attributes.h"
#include "libk/kprintf.h"
#include "types.h"

// https://github.com/llvm/llvm-project/blob/release/16.x/clang/lib/CodeGen/CodeGenFunction.h#L113*/

// EmitCheck
// https://github.com/llvm/llvm-project/blob/8ceba5980c45a7819186bfd699ba3723b9f8b6a0/clang/lib/CodeGen/CodeGenFunction.h#L4674

// EmitCheck source
// https://github.com/llvm/llvm-project/blob/b26dd42458806622e2bd56f6fc62149affe51219/clang/lib/CodeGen/CGExpr.cpp#L3282

// See: compiler-rt/lib/ubsan/ubsan_handlers.h
// See: compiler-rt/lib/ubsan/ubsan_handlers.c
typedef struct {
    const char *filename;
    u32 line;
    u32 column;
} source_location_t;

typedef struct {
    u16 type_kind;
    u16 type_info;
    char type_name[1];
} type_descriptor_t;

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
void __ubsan_handle_type_mismatch_v1(const type_mismatch_data_t *data,
                                     void *ptr) {
    kprintf("ubsan: type_mismatch_v1 in %s:%u:%u\n", data->loc.filename,
            data->loc.line, data->loc.column);

    u64 alignment = 1UL << data->log_alignment;
    if (ptr == NULL) {
        kprintf("%s null pointer of type %s\n",
                type_check_kinds[data->type_check_kind], data->type->type_name);
    } else if ((u64)ptr & (alignment - 1)) {
        kprintf(
            "%s misaligned address %lx for type %s, which requires %lu byte "
            "alignment\n",
            type_check_kinds[data->type_check_kind], (u64)ptr,
            data->type->type_name, alignment);
    } else {
        kprintf(
            "%s address %lx with insufficient space for an object of type %s\n",
            type_check_kinds[data->type_check_kind], (u64)ptr,
            data->type->type_name);
    }
}

// TODO: Pointer, Alignment, Offset
void __ubsan_handle_alignment_assumption(
    __unused const alignment_assumption_data_t *data) {
    kprintf("ubsan: alignment_assumption\n");
}

// TODO: LHS, RHS
void __ubsan_handle_add_overflow(__unused const overflow_data_t *data) {
    kprintf("ubsan: add_overflow\n");
}

// TODO: LHS, RHS
void __ubsan_handle_sub_overflow(__unused const overflow_data_t *data) {
    kprintf("ubsan: sub_overflow\n");
}

// TODO: LHS, RHS
void __ubsan_handle_mul_overflow(__unused const overflow_data_t *data) {
    kprintf("ubsan: mul_overflow\n");
}

// TODO: OldVal
void __ubsan_handle_negate_overflow(__unused const overflow_data_t *data) {
    kprintf("ubsan: negate_overflow\n");
}

// TODO: LHS, RHS
void __ubsan_handle_divrem_overflow(__unused const overflow_data_t *data) {
    kprintf("ubsan: divrem_overflow\n");
}

// TODO: LHS, RHS
void __ubsan_handle_shift_out_of_bounds(
    __unused const shift_out_of_bounds_data_t *data) {
    kprintf("ubsan: shift_out_of_bounds\n");
}

// TODO: Index
void __ubsan_handle_out_of_bounds(__unused const out_of_bounds_data_t *data) {
    kprintf("ubsan: out_of_bounds\n");
}

void __ubsan_handle_builtin_unreachable(
    __unused const unreachable_data_t *data) {
    kprintf("ubsan: builtin_unreachable\n");
}

void __ubsan_handle_missing_return(__unused const unreachable_data_t *data) {
    kprintf("ubsan: missing_return\n");
}

// TODO: Base, Result
void __ubsan_handle_pointer_overflow(
    __unused const pointer_overflow_data_t *data) {
    kprintf("ubsan: pointer_overflow\n");
}
