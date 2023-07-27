/*
 * This file contains GDT definition (loaded by boot.S) and tss loading
 * function.
 */

#include <stdint.h>

#include "gdt.h"
#include "utils.h"

struct segment_descriptor gdt[GDT_NUM_ENTRIES] __align(GDT_ALIGN) = {
    [0] = GDT_NULL_DESCRIPTOR,

    [GDT_IDX_CODE] = GDT_SEG_DESCRIPTOR(0, 0xfffff, SEG_TYPE_RE, SEG_DPL_RING_0,
                                        SEG_MODE_LONG, 0, SEG_GRAN_4KIB),
    [GDT_IDX_DATA] = GDT_SEG_DESCRIPTOR(0, 0xfffff, SEG_TYPE_RW, SEG_DPL_RING_0,
                                        0, 1, SEG_GRAN_4KIB),

    // I don't think we can initialize it cleanly at compile time since it isn't
    // the same size.
    [GDT_IDX_TSS] = GDT_NULL_DESCRIPTOR,
    [GDT_IDX_TSS_HI] = GDT_NULL_DESCRIPTOR,
};

static struct tss tss = { 0 };

void load_tss(void) {
    struct tss_descriptor *tss_descriptor = (void *)&gdt[GDT_IDX_TSS];
    *tss_descriptor =
        (struct tss_descriptor)TSS_DESCRIPTOR((uint64_t)&tss, sizeof(tss));

    uint16_t segment_selector =
        SEGMENT_SELECTOR(GDT_IDX_TSS, SEGMENT_SELECTOR_GDT, 0);
    __asm__ volatile("ltr %0\n" ::"m"(segment_selector));
}
