#include <stddef.h>

#include "acpi.h"
#include "libk/kassert.h"
#include "libk/kprintf.h"
#include "libk/log.h"
#include "libk/string.h"
#include "mm/vmm.h"
#include "utils.h"

rsdp_t g_rsdp;
u64 acpi_region_addr;
u64 acpi_region_len;

// This function does not init ACPI but stores ACPI informations to be able to
// use ACPI after pmm_init.
void acpi_prepare(const struct multiboot_tag_old_acpi *old_acpi_tag,
                  const struct multiboot_tag_mmap *mmap_tag) {
    kassert(old_acpi_tag->size == 28);

    // Copy RSDP to kernel range because RSDP is outside kernel range and it is
    // considered invalid after pmm_init.
    g_rsdp = *(const rsdp_t *)old_acpi_tag->rsdp;
    kassert(acpi_rsdp_is_valid_checksum(&g_rsdp));
    // Assert that we have an old ACPI RSDP
    kassert(g_rsdp.revision == 0);

    acpi_print_rsdp(&g_rsdp);

    bool acpi_region_found = false;
    for (u64 i = 0; sizeof(struct multiboot_tag_mmap)
             + i * sizeof(struct multiboot_mmap_entry)
         < mmap_tag->size;
         i++) {
        if (g_rsdp.rsdt_phys_addr >= mmap_tag->entries[i].addr
            && g_rsdp.rsdt_phys_addr
                < mmap_tag->entries[i].addr + mmap_tag->entries[i].len) {
            log(LOG_LEVEL_DEBUG, "RSDT region type: %u\n",
                mmap_tag->entries[i].type);
            acpi_region_found = true;
            acpi_region_addr = mmap_tag->entries[i].addr;
            acpi_region_len = mmap_tag->entries[i].len;
            break;
        }
    }
    kassert(acpi_region_found);
}

u64 acpi_map_region(void) {
    return vmm_map_physical(ACPI_VIRT_ADDR, acpi_region_addr, acpi_region_len,
                            0);
}

// Returns NULL if not found
const description_header_t *acpi_rsdt_find_table(char signature[4]) {
    const rsdt_t *rsdt = (void *)ACPI_PHYS_TO_VIRT(g_rsdp.rsdt_phys_addr);

    const description_header_t *description_header = NULL;
    for (u64 i = 0;
         i * sizeof(u32) + offsetof(rsdt_t, entry) < rsdt->header.length; ++i) {
        kassert(rsdt->entry[i] > acpi_region_addr
                && rsdt->entry[i] < acpi_region_addr + acpi_region_len);

        description_header =
            (void *)(rsdt->entry[i] - acpi_region_addr + ACPI_VIRT_ADDR);

        if (strncmp(description_header->signature, signature, 4) == 0) {
            kassert(acpi_table_is_valid_checksum(description_header));
            return description_header;
        }
    }

    return NULL;
}

// Returns 0xffffffff if no IO APIC structure is found
u32 acpi_madt_find_ioapic_addr(const madt_t *madt) {
    const madt_int_controller_t *int_controller = &madt->int_controllers;
    while ((u64)int_controller - (u64)madt < madt->header.length) {
        if (int_controller->type == MADT_INT_CONTROLLER_IOAPIC) {
            const madt_int_controller_ioapic_t *ic = (void *)int_controller;
            return ic->ioapic_phys_addr;
        }

        // WARN: Here we assume that the length is the second byte for all
        // the variant (which is true for ACPI 6.5)
        u8 int_controller_len = *((u8 *)int_controller + 1);
        int_controller = (void *)((u64)int_controller + int_controller_len);
    }

    return 0xffffffff;
}

bool acpi_rsdp_is_valid_checksum(const rsdp_t *rsdp) {
    u8 sum = 0;
    for (u32 i = 0; i < sizeof(rsdp_t); ++i) {
        sum += ((const u8 *)rsdp)[i];
    }

    return sum == 0;
}

bool acpi_table_is_valid_checksum(const description_header_t *header) {
    u8 sum = 0;
    for (u32 i = 0; i < header->length; ++i) {
        sum += ((const u8 *)header)[i];
    }

    return sum == 0;
}

void acpi_print_rsdp(const rsdp_t *rsdp) {
    puts("RSDP:\n");

    kprintf("  Signature: %c%c%c%c%c%c%c%c\n", rsdp->signature[0],
            rsdp->signature[1], rsdp->signature[2], rsdp->signature[3],
            rsdp->signature[4], rsdp->signature[5], rsdp->signature[6],
            rsdp->signature[7]);
    kprintf("  Checksum: 0x%02hhx\n", rsdp->checksum);
    kprintf("  OEM ID: %c%c%c%c%c%c\n", rsdp->oem_id[0], rsdp->oem_id[1],
            rsdp->oem_id[2], rsdp->oem_id[3], rsdp->oem_id[4], rsdp->oem_id[5]);
    kprintf("  Revision: %u\n", rsdp->revision);
    kprintf("  RSDT physical address: 0x%08x\n", rsdp->rsdt_phys_addr);
}

static void acpi_print_description_header(const description_header_t *header) {
    kprintf("  Signature: %c%c%c%c\n", header->signature[0],
            header->signature[1], header->signature[2], header->signature[3]);
    kprintf("  Length: %u\n", header->length);
    kprintf("  Revision: %u\n", header->revision);
    kprintf("  Checksum: 0x%02hhx\n", header->checksum);
    kprintf("  OEM ID: %c%c%c%c%c%c\n", header->oem_id[0], header->oem_id[1],
            header->oem_id[2], header->oem_id[3], header->oem_id[4],
            header->oem_id[5]);
    kprintf("  OEM Table ID: %c%c%c%c%c%c%c%c\n", header->oem_table_id[0],
            header->oem_table_id[1], header->oem_table_id[2],
            header->oem_table_id[3], header->oem_table_id[4],
            header->oem_table_id[5], header->oem_table_id[6],
            header->oem_table_id[7]);
    kprintf("  OEM Revision: 0x%08x\n", header->oem_revision);
    kprintf("  Creator ID: %u\n", header->creator_id);
    kprintf("  Creator Revision: 0x%08x\n", header->creator_revision);
}

void acpi_print_madt(const madt_t *madt) {
    puts("MADT:\n");
    acpi_print_description_header(&madt->header);
    kprintf("  Local interrupt controller address: 0x%08x\n",
            madt->lapic_phys_addr);
    kprintf("  Flags: 0x%08x\n", madt->flags);

    const madt_int_controller_t *int_controller = &madt->int_controllers;
    while ((u64)int_controller - (u64)madt < madt->header.length) {
        kprintf("  Interrupt controller type: 0x%02x\n", int_controller->type);

        if (int_controller->type == MADT_INT_CONTROLLER_LAPIC) {
            const madt_int_controller_lapic_t *ic = (void *)int_controller;
            kprintf("    Length: %u\n", ic->length);
            kprintf("    ACPI Processor UID: %u\n", ic->acpi_processor_id);
            kprintf("    APIC ID: %u\n", ic->apic_id);
            kprintf("    Flags: %u\n", ic->flags);
        } else if (int_controller->type == MADT_INT_CONTROLLER_IOAPIC) {
            const madt_int_controller_ioapic_t *ic = (void *)int_controller;
            kprintf("    Length: %u\n", ic->length);
            kprintf("    IO APIC ID: %u\n", ic->ioapic_id);
            kprintf("    IO APIC Address: 0x%08x\n", ic->ioapic_phys_addr);
            kprintf("    Global System Interrupt Base: 0x%08x\n",
                    ic->system_vector_base);
        } else if (int_controller->type
                   == MADT_INT_CONTROLLER_INT_SRC_OVERRIDE) {
            const madt_int_controller_int_src_override_t *ic =
                (void *)int_controller;
            kprintf("    Length: %u\n", ic->length);
            kprintf("    Bus: %u\n", ic->bus);
            kprintf("    Source: %u\n", ic->source);
            kprintf("    Global System Interrupt: 0x%08x\n",
                    ic->global_system_interrupt);
            kprintf("    Flags: %u\n", ic->flags);
        } else if (int_controller->type == MADT_INT_CONTROLLER_LAPIC_NMI) {
            const madt_int_controller_lapic_nmi_t *ic = (void *)int_controller;
            kprintf("    Length: %u\n", ic->length);
            kprintf("    ACPI Processor UID: %u\n", ic->acpi_processor_id);
            kprintf("    Flags: %u\n", ic->flags);
            kprintf("    Local APIC LINT#: %u\n", ic->lapic_lint_num);
        }

        // WARN: Here we assume that the length is the second byte for all
        // the variant (which is true for ACPI 6.5)
        u8 int_controller_len = *((u8 *)int_controller + 1);
        int_controller = (void *)((u64)int_controller + int_controller_len);
    }
}

void acpi_print_hpet_description_table(const hpet_description_table_t *hpet) {
    puts("HPET:\n");

    kprintf("  Event timer block id: 0x%08x\n", hpet->event_timer_block_id);
    kprintf("  Base address: 0x%016lx\n", hpet->base_address.addr);
    kprintf("  HPET number: %u\n", hpet->hpet_num);
    kprintf("  Main counter minimum clock ticks: %u\n", hpet->min_ticks);
    kprintf("  Attributes: 0x%02hhx\n", hpet->attrs);
}
