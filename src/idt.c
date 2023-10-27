#include "attributes.h"
#include "gdt.h"
#include "idt.h"
#include "isr.h"
#include "types.h"
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
typedef struct {
    u16 offset_lo;
    u16 segment;
    u16 ist : 3;
    u16 : 5;
    u16 type : 4;
    u16 : 1;
    u16 dpl : 2;
    u16 present : 1;
    u16 offset_mid;
    u32 offset_hi;
    u32 reserved;
} __packed gate_descriptor_t;

#define GATE_TYPE_INT 0b1110
#define GATE_TYPE_TRAP 0b1111

#define GATE_DESCRIPTOR(OFFSET, TYPE)                                          \
    (gate_descriptor_t) {                                                      \
        .offset_lo = (OFFSET)&0xffff,                                          \
        .segment = SEGMENT_SELECTOR(GDT_IDX_CODE, SEGMENT_SELECTOR_GDT, 0),    \
        .ist = 0, .type = TYPE, .dpl = 0, .present = 1,                        \
        .offset_mid = ((OFFSET) >> 16) & 0xffff,                               \
        .offset_hi = (u32)((OFFSET) >> 32) & 0xffffffff,                       \
    }

static gate_descriptor_t idt[256] __align(IDT_ALIGN) = { 0 };

void load_idt(void) {
    idt[0] = GATE_DESCRIPTOR((u64)isr_0, GATE_TYPE_INT);
    idt[1] = GATE_DESCRIPTOR((u64)isr_1, GATE_TYPE_INT);
    idt[2] = GATE_DESCRIPTOR((u64)isr_2, GATE_TYPE_INT);
    idt[3] = GATE_DESCRIPTOR((u64)isr_3, GATE_TYPE_INT);
    idt[4] = GATE_DESCRIPTOR((u64)isr_4, GATE_TYPE_INT);
    idt[5] = GATE_DESCRIPTOR((u64)isr_5, GATE_TYPE_INT);
    idt[6] = GATE_DESCRIPTOR((u64)isr_6, GATE_TYPE_INT);
    idt[7] = GATE_DESCRIPTOR((u64)isr_7, GATE_TYPE_INT);
    idt[8] = GATE_DESCRIPTOR((u64)isr_8, GATE_TYPE_INT);
    idt[10] = GATE_DESCRIPTOR((u64)isr_10, GATE_TYPE_INT);
    idt[11] = GATE_DESCRIPTOR((u64)isr_11, GATE_TYPE_INT);
    idt[12] = GATE_DESCRIPTOR((u64)isr_12, GATE_TYPE_INT);
    idt[13] = GATE_DESCRIPTOR((u64)isr_13, GATE_TYPE_INT);
    idt[14] = GATE_DESCRIPTOR((u64)isr_14, GATE_TYPE_INT);
    idt[16] = GATE_DESCRIPTOR((u64)isr_16, GATE_TYPE_INT);
    idt[17] = GATE_DESCRIPTOR((u64)isr_17, GATE_TYPE_INT);
    idt[18] = GATE_DESCRIPTOR((u64)isr_18, GATE_TYPE_INT);
    idt[19] = GATE_DESCRIPTOR((u64)isr_19, GATE_TYPE_INT);
    idt[21] = GATE_DESCRIPTOR((u64)isr_21, GATE_TYPE_INT);
    idt[32] = GATE_DESCRIPTOR((u64)isr_32, GATE_TYPE_INT);
    idt[33] = GATE_DESCRIPTOR((u64)isr_33, GATE_TYPE_INT);
    idt[34] = GATE_DESCRIPTOR((u64)isr_34, GATE_TYPE_INT);
    idt[35] = GATE_DESCRIPTOR((u64)isr_35, GATE_TYPE_INT);
    idt[36] = GATE_DESCRIPTOR((u64)isr_36, GATE_TYPE_INT);
    idt[37] = GATE_DESCRIPTOR((u64)isr_37, GATE_TYPE_INT);
    idt[38] = GATE_DESCRIPTOR((u64)isr_38, GATE_TYPE_INT);
    idt[39] = GATE_DESCRIPTOR((u64)isr_39, GATE_TYPE_INT);
    idt[40] = GATE_DESCRIPTOR((u64)isr_40, GATE_TYPE_INT);
    idt[41] = GATE_DESCRIPTOR((u64)isr_41, GATE_TYPE_INT);
    idt[42] = GATE_DESCRIPTOR((u64)isr_42, GATE_TYPE_INT);
    idt[43] = GATE_DESCRIPTOR((u64)isr_43, GATE_TYPE_INT);
    idt[44] = GATE_DESCRIPTOR((u64)isr_44, GATE_TYPE_INT);
    idt[45] = GATE_DESCRIPTOR((u64)isr_45, GATE_TYPE_INT);
    idt[46] = GATE_DESCRIPTOR((u64)isr_46, GATE_TYPE_INT);
    idt[47] = GATE_DESCRIPTOR((u64)isr_47, GATE_TYPE_INT);
    idt[48] = GATE_DESCRIPTOR((u64)isr_48, GATE_TYPE_INT);
    idt[49] = GATE_DESCRIPTOR((u64)isr_49, GATE_TYPE_INT);
    idt[50] = GATE_DESCRIPTOR((u64)isr_50, GATE_TYPE_INT);
    idt[51] = GATE_DESCRIPTOR((u64)isr_51, GATE_TYPE_INT);
    idt[52] = GATE_DESCRIPTOR((u64)isr_52, GATE_TYPE_INT);
    idt[53] = GATE_DESCRIPTOR((u64)isr_53, GATE_TYPE_INT);
    idt[54] = GATE_DESCRIPTOR((u64)isr_54, GATE_TYPE_INT);
    idt[55] = GATE_DESCRIPTOR((u64)isr_55, GATE_TYPE_INT);
    idt[56] = GATE_DESCRIPTOR((u64)isr_56, GATE_TYPE_INT);
    idt[57] = GATE_DESCRIPTOR((u64)isr_57, GATE_TYPE_INT);
    idt[58] = GATE_DESCRIPTOR((u64)isr_58, GATE_TYPE_INT);
    idt[59] = GATE_DESCRIPTOR((u64)isr_59, GATE_TYPE_INT);
    idt[60] = GATE_DESCRIPTOR((u64)isr_60, GATE_TYPE_INT);
    idt[61] = GATE_DESCRIPTOR((u64)isr_61, GATE_TYPE_INT);
    idt[62] = GATE_DESCRIPTOR((u64)isr_62, GATE_TYPE_INT);
    idt[63] = GATE_DESCRIPTOR((u64)isr_63, GATE_TYPE_INT);

    pseudo_descriptor64_t idt_descriptor = {
        .size = sizeof(idt) - 1,
        .offset = (u64)idt,
    };

    __asm__ volatile("lidt %0\n" ::"m"(idt_descriptor));
}
