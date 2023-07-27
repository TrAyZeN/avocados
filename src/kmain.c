#include <stddef.h>
#include <stdnoreturn.h>

#include "acpi.h"
#include "attributes.h"
#include "avocados.h"
#include "backtrace.h"
#include "framebuffer.h"
#include "gdt.h"
#include "idt.h"
#include "instr.h"
#include "kassert.h"
#include "kprintf.h"
#include "log.h"
#include "multiboot2.h"
#include "multiboot_utils.h"
#include "panic.h"
#include "pmm.h"
#include "serial.h"
#include "test.h"
#include "vmm.h"

#define MAGIC_BREAKPOINT __asm__ volatile("xchgw %bx, %bx")

uint8_t kernel_stack[KERNEL_STACK_SIZE] __align(16);

noreturn void kmain(multiboot_uint32_t magic, uint64_t addr) {
    serial_init(SERIAL_PORT_COM1, SERIAL_BAUDRATE_38400);

    run_tests();

    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
        kpanic("Invalid multiboot2 magic: %x\n", magic);
    }

    struct multiboot_tag *tag =
        multiboot_find_tag((void *)(addr + 8), MULTIBOOT_TAG_TYPE_FRAMEBUFFER);
    if (tag->type == MULTIBOOT_TAG_TYPE_END) {
        kpanic("No framebuffer info provided by the bootloader\n");
    }

    struct multiboot_tag_framebuffer *frame_buffer_tag = (void *)tag;
    if (frame_buffer_tag->common.framebuffer_type
        == MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT) {
        fb_init((void *)frame_buffer_tag->common.framebuffer_addr,
                frame_buffer_tag->common.framebuffer_width,
                frame_buffer_tag->common.framebuffer_height);
    } else {
        kpanic("Invalid framebuffer type: %u\n",
               frame_buffer_tag->common.framebuffer_type);
    }

    load_tss();
    load_idt();

    tag = multiboot_find_tag((void *)(addr + 8), MULTIBOOT_TAG_TYPE_MMAP);
    if (tag->type == MULTIBOOT_TAG_TYPE_END) {
        kpanic("No memory map provided by the bootloader");
    }

    struct multiboot_tag_mmap *mmap_tag = (void *)tag;
    multiboot_print_mmap(mmap_tag);

    tag = multiboot_find_tag((void *)(addr + 8), MULTIBOOT_TAG_TYPE_ACPI_OLD);
    if (tag->type == MULTIBOOT_TAG_TYPE_END) {
        kpanic("No old ACPI\n");
    }
    /* tag = multiboot_find_tag((void *)(addr + 8),
     * MULTIBOOT_TAG_TYPE_ACPI_NEW); */
    /* if (tag->type == MULTIBOOT_TAG_TYPE_END) { */
    /* kpanic("No new ACPI\n"); */
    /* } */
    struct multiboot_tag_old_acpi *old_acpi_tag = (void *)tag;
    kassert(old_acpi_tag->size == 28);

    const struct rsdp *rsdp = (void *)old_acpi_tag->rsdp;

    uint8_t sum = 0;
    for (uint64_t i = 0; i < sizeof(struct rsdp); ++i) {
        sum += ((uint8_t *)rsdp)[i];
    }
    kassert(sum == 0);
    kassert(rsdp->revision == 0);

    acpi_print_rsdp(rsdp);

    bool rsdt_region_found = false;
    uint64_t rsdt_region_addr = 0;
    uint64_t rsdt_region_len = 0;
    for (uint64_t i = 0; sizeof(struct multiboot_tag_mmap)
             + i * sizeof(struct multiboot_mmap_entry)
         < mmap_tag->size;
         i++) {
        if (rsdp->rsdt_phys_addr >= mmap_tag->entries[i].addr
            && rsdp->rsdt_phys_addr
                < mmap_tag->entries[i].addr + mmap_tag->entries[i].len) {
            log(LOG_LEVEL_DEBUG, "RSDT region type: %u\n",
                mmap_tag->entries[i].type);
            rsdt_region_found = true;
            rsdt_region_addr = mmap_tag->entries[i].addr;
            rsdt_region_len = mmap_tag->entries[i].len;
            break;
        }
    }
    kassert(rsdt_region_found);

    backtrace();

    pmm_init(mmap_tag);
    vmm_init();

    (void)rsdt_region_addr;
    (void)rsdt_region_len;

    uint64_t res = vmm_map_physical(0x0000001000000000UL, rsdt_region_addr,
                                    rsdt_region_len, 0);
    kassert(res != VMM_ALLOC_ERROR);

    uint64_t mmap_addr = vmm_alloc(0xffffb33333333000UL, VMM_ALLOC_RW);
    kassert(mmap_addr != VMM_ALLOC_ERROR);
    kprintf("mmap_addr: %lx\n", mmap_addr);
    vmm_free(mmap_addr);

    MAGIC_BREAKPOINT;

    __asm__ volatile("int $0\n");
    __asm__ volatile("int $0\n");

    /* sti(); */

    // TODO: Add documentation
    // TODO: Higher half kernel
    // TODO: vmm

    // TODO: APIC
    // TODO: HPET

    // TODO: GDB stub
    // TODO: Unwind
    // TODO: ubsan
    // https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=c6d308534aef6c99904bf5862066360ae067abc4

    /* end: */
    kpanic("End of kmain reached\n");

    cli();
    while (1) {
        hlt();
    }
}
