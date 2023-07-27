#include <stdint.h>

#include "attributes.h"
#include "gdt.h"
#include "idt.h"
#include "isr.h"
#include "utils.h"

// The base addresses of the IDT should be aligned on an 8-byte boundary to
// maximize performance of cache line fills. (Vol. 3A 6.10)
#define IDT_ALIGN 8

/*
 * TODO:
 * - offset: Offset to the procedure entry point.
 * - segment: Segment selector of the target code segment.
 * - ist: Interrupt Stack Table used by the stack switching mechanism
 * - type:
 * - dpl:
 */
struct gate_descriptor {
    uint16_t offset_lo;
    uint16_t segment;
    uint16_t ist : 3;
    uint16_t : 5;
    uint16_t type : 4;
    uint16_t : 1;
    uint16_t dpl : 2;
    uint16_t present : 1;
    uint16_t offset_mid;
    uint32_t offset_hi;
    uint32_t reserved;
} __packed;

#define GATE_TYPE_INT 0b1110
#define GATE_TYPE_TRAP 0b1111

#define GATE_DESCRIPTOR(OFFSET, TYPE)                                          \
    (struct gate_descriptor) {                                                 \
        .offset_lo = (OFFSET)&0xffff,                                          \
        .segment = SEGMENT_SELECTOR(GDT_IDX_CODE, SEGMENT_SELECTOR_GDT, 0),    \
        .ist = 0, .type = TYPE, .dpl = 0, .present = 1,                        \
        .offset_mid = ((OFFSET) >> 16) & 0xffff,                               \
        .offset_hi = (uint32_t)((OFFSET) >> 32) & 0xffffffff,                  \
    }

static struct gate_descriptor idt[256] __align(IDT_ALIGN) = { 0 };

void load_idt(void) {
    idt[0] = GATE_DESCRIPTOR((uint64_t)isr_0, GATE_TYPE_INT);
    idt[1] = GATE_DESCRIPTOR((uint64_t)isr_1, GATE_TYPE_INT);
    idt[2] = GATE_DESCRIPTOR((uint64_t)isr_2, GATE_TYPE_INT);
    idt[3] = GATE_DESCRIPTOR((uint64_t)isr_3, GATE_TYPE_INT);
    idt[4] = GATE_DESCRIPTOR((uint64_t)isr_4, GATE_TYPE_INT);
    idt[5] = GATE_DESCRIPTOR((uint64_t)isr_5, GATE_TYPE_INT);
    idt[6] = GATE_DESCRIPTOR((uint64_t)isr_6, GATE_TYPE_INT);
    idt[7] = GATE_DESCRIPTOR((uint64_t)isr_7, GATE_TYPE_INT);
    idt[8] = GATE_DESCRIPTOR((uint64_t)isr_8, GATE_TYPE_INT);
    idt[10] = GATE_DESCRIPTOR((uint64_t)isr_10, GATE_TYPE_INT);
    idt[11] = GATE_DESCRIPTOR((uint64_t)isr_11, GATE_TYPE_INT);
    idt[12] = GATE_DESCRIPTOR((uint64_t)isr_12, GATE_TYPE_INT);
    idt[13] = GATE_DESCRIPTOR((uint64_t)isr_13, GATE_TYPE_INT);
    idt[14] = GATE_DESCRIPTOR((uint64_t)isr_14, GATE_TYPE_INT);
    idt[16] = GATE_DESCRIPTOR((uint64_t)isr_16, GATE_TYPE_INT);
    idt[17] = GATE_DESCRIPTOR((uint64_t)isr_17, GATE_TYPE_INT);
    idt[18] = GATE_DESCRIPTOR((uint64_t)isr_18, GATE_TYPE_INT);
    idt[19] = GATE_DESCRIPTOR((uint64_t)isr_19, GATE_TYPE_INT);
    idt[21] = GATE_DESCRIPTOR((uint64_t)isr_21, GATE_TYPE_INT);

    struct pseudo_descriptor64 idt_descriptor = {
        .size = sizeof(idt) - 1,
        .offset = (uint64_t)idt,
    };

    __asm__ volatile("lidt %0\n" ::"m"(idt_descriptor));
}
