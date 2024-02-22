#ifndef AVOCADOS_UTILS_H_
#define AVOCADOS_UTILS_H_

// A segment selector is a 16-bit identifier for a segment.
// - IDX: Index of the segment descriptor in the descriptor table.
// - TI: Specifies the descriptor table to use (0 = GDT, 1 = GDT).
// - RPL: Specifies the privilege level of the selector.
// See Vol. 3A 3.4.2.
#define SEGMENT_SELECTOR(IDX, TI, RPL)                                         \
    (((IDX)&0x1fff) << 3 | ((TI)&1) | ((RPL)&0b11))

#define SEGMENT_SELECTOR_GDT 0
#define SEGMENT_SELECTOR_LDT 1

#ifndef __ASSEMBLER__

#include "attributes.h"
#include "types.h"

// Descriptor stored in GDTR, IDTR and LTDR.
// See Vol. 3A 3.5.1
typedef struct {
    u16 size;
    u64 offset;
} __packed pseudo_descriptor64_t;

// ALIGN must be a power of two
/* #define ALIGN_UP(VALUE, ALIGN) ((VALUE) & ~((ALIGN)-1)) */
// align VALUE to the next multiple of ALIGN
// TODO: Inline function
#define ALIGN_UP(VALUE, ALIGN) ((((VALUE) + (ALIGN)-1U) / (ALIGN)) * (ALIGN))
#define ALIGN_DOWN(VALUE, ALIGN) (((VALUE) / (ALIGN)) * (ALIGN))
_Static_assert(ALIGN_UP(4096, 4096) == 4096, "ALIGN_UP test");
_Static_assert(ALIGN_DOWN(4096, 4096) == 4096, "ALIGN_DOWN test");

// TODO: Assert that START <= END
// WARN: The problem of converting this to a function is the type range
// We need a generic macro
// Returns bits of val in the range start to end included (indexed from lsb).
#define BIT_RANGE(VAL, START, END)                                             \
    (((VAL) >> (START)) & ((1UL << ((END) - (START) + 1U)) - 1UL))

// TODO: Assert that START <= END
#define BIT_BLOCK(VAL, START, END)                                             \
    (((VAL) & ((1UL << ((END) - (START) + 1U)) - 1UL)) << (START))

#endif /* ! __ASSEMBLER__ */

#endif /* ! AVOCADOS_UTILS_H_ */
