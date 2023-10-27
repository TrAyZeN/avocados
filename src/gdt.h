#ifndef AVOCADOS_GDT_H_
#define AVOCADOS_GDT_H_

#define GDT_NUM_ENTRIES 6
#define GDT_ENTRY_SIZE 8
// The base address of the GDT should be aligned on an eight-byte boundary to
// yield the best processor performance. (Vol. 3A 3.5.1)
#define GDT_ALIGN 8

#define GDT_IDX_CODE 1
#define GDT_IDX_DATA 2
#define GDT_IDX_TSS 4
#define GDT_IDX_TSS_HI (GDT_IDX_TSS + 1)

#define BYTES_GDT_SEG_DESCRIPTOR(BASE, LIMIT, TYPE, DPL, LONG_MODE, DB,        \
                                 GRANULARITY)                                  \
    (((LIMIT)&0xffff) | (((BASE)&0xffffff) << 16) | ((TYPE) << 40)             \
     | (SEG_NO_SYS << 44) | ((DPL) << 45) | (SEG_PRESENT << 47)                \
     | ((((LIMIT) >> 16) & 0xf) << 48) | ((LONG_MODE) << 53) | ((DB) << 54)    \
     | ((GRANULARITY) << 55) | ((((BASE) >> 24) & 0xff) << 56))

#define SEG_TYPE_R 0
#define SEG_TYPE_RW (SEG_TYPE_R | (1 << 1))
#define SEG_TYPE_E (1 << 3)
#define SEG_TYPE_RE (SEG_TYPE_E | (1 << 1))
#define SEG_TYPE_TSS_AVL 0b1001

#define SEG_NO_SYS 1

#define SEG_DPL_RING_0 0
#define SEG_DPL_RING_3 3

#define SEG_PRESENT 1

#define SEG_MODE_LONG 1

#define SEG_GRAN_1B 0
#define SEG_GRAN_4KIB 1

#ifndef __ASSEMBLER__

#include "attributes.h"
#include "types.h"

/*
 * Segment descriptor format (see Vol. 3A 3.4.5):
 * - limit: Specifies the size of the segment
 * - base: Defines the location of byte 0 of the segment
 * - type:
 *   0 Accessed bit
 *   1 Write enable
 *   2 Direction bit/Conforming bit
 *   3 Executable: If set defines a code segment else data segment.
 * - system: If clear indicates a system segment else a code or data
 * segment.
 * - dpl: Specifies the privilege level of the segment. It is used to
 * control access to segment.
 * - present: Indicates whether the segment is present in memory.
 * - long_mode: If set the descriptor defines 64-bit code segment. Also if
 * set db must be 0.
 * - db: If clear the descriptor defines 16-bit protected mode segment else
 *   32-bit protected mode segment.
 * - gran: If set the granularity of the limit is 4096B else 1B.
 */
typedef struct {
    u32 limit_lo : 16;
    u32 base_lo : 24;
    u32 type : 4;
    u32 system : 1;
    u32 dpl : 2;
    u32 present : 1;
    u32 limit_hi : 4;
    u32 : 1;
    u32 long_mode : 1;
    u32 db : 1;
    u32 gran : 1;
    u32 base_hi : 8;
} __packed segment_descriptor_t;
_Static_assert(sizeof(segment_descriptor_t) == 8,
               "Invalid segment descriptor size");

#define GDT_NULL_DESCRIPTOR                                                    \
    { 0 }

#define GDT_SEG_DESCRIPTOR(BASE, LIMIT, TYPE, DPL, LONG_MODE, DB, GRANULARITY) \
    {                                                                          \
        .limit_lo = (LIMIT)&0xffff, .base_lo = (BASE)&0xffffff, .type = TYPE,  \
        .system = SEG_NO_SYS, .dpl = DPL, .present = SEG_PRESENT,              \
        .limit_hi = ((LIMIT) >> 16) & 0xf, .long_mode = LONG_MODE, .db = DB,   \
        .gran = GRANULARITY, .base_hi = ((BASE) >> 24) & 0xff,                 \
    }

// Vol. 3A 8.2.3
typedef struct {
    u32 limit_lo : 16;
    u32 base_lo : 24;
    u32 type : 4;
    u32 : 1;
    u32 dpl : 2;
    u32 present : 1;
    u32 limit_hi : 4;
    u32 : 3;
    u32 gran : 1;
    u32 base_mid : 8;
    u32 base_hi;
    u32 : 32;
} __packed tss_descriptor_t;
_Static_assert(sizeof(tss_descriptor_t) == 16, "Invalid TSS descriptor size");

#define TSS_DESCRIPTOR(BASE, LIMIT)                                            \
    {                                                                          \
        .limit_lo = (LIMIT)&0xffff, .base_lo = (BASE)&0xffffff,                \
        .type = SEG_TYPE_TSS_AVL, .dpl = SEG_DPL_RING_0,                       \
        .present = SEG_PRESENT, .limit_hi = ((LIMIT) >> 16) & 0xf,             \
        .gran = SEG_GRAN_1B, .base_mid = ((BASE) >> 24) & 0xff,                \
        .base_hi = (u32)((BASE) >> 32) & 0xffffffff,                           \
    }

// A 64-bit TSS holds the following information that is important to 64-bit
// operation:
// - Stack pointer addresses for each privilege level
// - Pointer addresses for the interrupt stack table
// - Offset address of the IO-permission bitmap
// Vol. 3A 2.1.3.1
typedef struct {
    u32 : 32;
    u32 rsp0_lo;
    u32 rsp0_hi;
    u32 rsp1_lo;
    u32 rsp1_hi;
    u32 rsp2_lo;
    u32 rsp2_hi;
    u32 : 32;
    u32 : 32;
    u32 ist1_lo;
    u32 ist1_hi;
    u32 ist2_lo;
    u32 ist2_hi;
    u32 ist3_lo;
    u32 ist3_hi;
    u32 ist4_lo;
    u32 ist4_hi;
    u32 ist5_lo;
    u32 ist5_hi;
    u32 ist6_lo;
    u32 ist6_hi;
    u32 ist7_lo;
    u32 ist7_hi;
    u32 : 32;
    u32 : 32;
    u16 : 16;
    u16 io_map_base;
} __packed tss_t;

void load_tss(void);

#endif /* ! __ASSEMBLER__ */

#endif /* ! AVOCADOS_GDT_H_ */
