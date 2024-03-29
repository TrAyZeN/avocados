#include <stddef.h>
#include <stdnoreturn.h>

#include "arch/gdt.h"
#include "arch/idt.h"
#include "arch/instr.h"
#include "attributes.h"
#include "avocados.h"
#include "backtrace.h"
#include "drivers/acpi.h"
#include "drivers/apic.h"
#include "drivers/framebuffer.h"
#include "drivers/hpet.h"
#include "drivers/pci.h"
#include "drivers/serial.h"
#include "libk/kassert.h"
#include "libk/kprintf.h"
#include "libk/log.h"
#include "libk/panic.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "multiboot2.h"
#include "multiboot_utils.h"
#include "tools/test.h"
#include "utils.h"

#define MAGIC_BREAKPOINT __asm__ volatile("xchgw %bx, %bx")

u8 kernel_stack[KERNEL_STACK_SIZE] __align(16);

noreturn void kmain(multiboot_uint32_t magic, u64 multiboot_info_addr) {
    serial_init(SERIAL_PORT_COM1, SERIAL_BAUDRATE_38400);

    run_tests();

    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
        kpanic("Invalid multiboot2 magic: 0x%08x\n", magic);
    }
    // A single mapping for multiboot_info_addr is created assuming it fits on a
    // single page frame
    kassert(*(u32 *)multiboot_info_addr <= 4096);
    kassert(multiboot_info_addr % 4096 == 0);

    const struct multiboot_tag *tag = multiboot_find_tag(
        (void *)(multiboot_info_addr + 8), MULTIBOOT_TAG_TYPE_FRAMEBUFFER);
    if (tag->type == MULTIBOOT_TAG_TYPE_END) {
        kpanic("No framebuffer info provided by the bootloader\n");
    }

    fb_prepare((struct multiboot_tag_framebuffer *)tag);

    load_tss();
    load_idt();

    tag = multiboot_find_tag((void *)(multiboot_info_addr + 8),
                             MULTIBOOT_TAG_TYPE_MMAP);
    if (tag->type == MULTIBOOT_TAG_TYPE_END) {
        kpanic("No memory map provided by the bootloader");
    }

    const struct multiboot_tag_mmap *mmap_tag = (void *)tag;
    multiboot_print_mmap(mmap_tag);

    tag = multiboot_find_tag((void *)(multiboot_info_addr + 8),
                             MULTIBOOT_TAG_TYPE_ACPI_OLD);
    if (tag->type == MULTIBOOT_TAG_TYPE_END) {
        kpanic("No old ACPI\n");
    }
    const struct multiboot_tag_old_acpi *old_acpi_tag = (void *)tag;

    tag = multiboot_find_tag((void *)(multiboot_info_addr + 8),
                             MULTIBOOT_TAG_TYPE_ACPI_NEW);
    if (tag->type == MULTIBOOT_TAG_TYPE_END) {
        log(LOG_LEVEL_INFO, "No new ACPI tag found\n");
    } else {
        log(LOG_LEVEL_INFO, "New ACPI tag found\n");
    }

    acpi_prepare(old_acpi_tag, mmap_tag);

    pmm_init(mmap_tag);
    vmm_init();

    kassert(fb_init() == 0);

    u64 res = acpi_map_region();
    kassert(res != VMM_ALLOC_ERROR);

    const madt_t *madt = (void *)acpi_rsdt_find_table("APIC");
    kassert(madt != NULL);
    acpi_print_madt(madt);

    u32 ioapic_phys_addr = acpi_madt_find_ioapic_addr(madt);
    kassert(ioapic_phys_addr != 0xffffffff);
    apic_init(madt->lapic_phys_addr, ioapic_phys_addr, (madt->flags & 1) == 1);

    const hpet_description_table_t *hpet = (void *)acpi_rsdt_find_table("HPET");
    kassert(hpet != NULL);
    acpi_print_hpet_description_table(hpet);

    hpet_init(hpet->base_address.addr);

    u64 mmap_addr = vmm_alloc(0xffffb33333333000UL, VMM_ALLOC_RW);
    kassert(mmap_addr != VMM_ALLOC_ERROR);
    kprintf("mmap_addr: %lx\n", mmap_addr);
    vmm_free(mmap_addr);

    /* backtrace(); */

    pci_list();

    MAGIC_BREAKPOINT;

    __asm__ volatile("int $0\n");
    __asm__ volatile("int $0\n");

    /* sti(); */

    // TODO: Set iopl

    // TODO: kmalloc

    // TODO: Add documentation
    // TODO: Higher half kernel

    // TODO: GDB stub
    // TODO: Unwind
    // TODO: ubsan
    // https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=c6d308534aef6c99904bf5862066360ae067abc4

    /* end: */

    puts("End of kmain reached\n");
    cli();
    while (1) {
        hlt();
    }
}
