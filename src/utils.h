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

#include <stdint.h>

#include "attributes.h"

// Descriptor stored in GDTR, IDTR and LTDR.
// See Vol. 3A 3.5.1
struct pseudo_descriptor64 {
    uint16_t size;
    uint64_t offset;
} __packed;

void memset(uint8_t *mem, uint8_t value, uint64_t n);
int strncmp(const char *s1, const char *s2, uint64_t n);

// ALIGN must be a power of two
/* #define ALIGN_UP(VALUE, ALIGN) ((VALUE) & ~((ALIGN)-1)) */
// align VALUE to the next multiple of ALIGN
#define ALIGN_UP(VALUE, ALIGN) ((((VALUE) + (ALIGN)-1U) / (ALIGN)) * (ALIGN))
#define ALIGN_DOWN(VALUE, ALIGN) (((VALUE) / (ALIGN)) * (ALIGN))
_Static_assert(ALIGN_UP(4096, 4096) == 4096, "ALIGN_UP test");
_Static_assert(ALIGN_DOWN(4096, 4096) == 4096, "ALIGN_DOWN test");

// Returns bits of val in the range start to end included (indexed from lsb).
#define BIT_RANGE(VAL, START, END)                                             \
    (((VAL) >> (START)) & ((1UL << ((END) - (START) + 1U)) - 1UL))

#endif /* ! __ASSEMBLER__ */

#endif /* ! AVOCADOS_UTILS_H_ */
