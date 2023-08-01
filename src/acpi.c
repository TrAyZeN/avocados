#include <stddef.h>

#include "acpi.h"
#include "kassert.h"
#include "kprintf.h"
#include "log.h"
#include "utils.h"
#include "vmm.h"

struct rsdp g_rsdp;
uint64_t acpi_region_addr;
uint64_t acpi_region_len;

// This function does not init ACPI but stores ACPI informations to be able to
// use ACPI after pmm_init.
void acpi_prepare(const struct multiboot_tag_old_acpi *old_acpi_tag,
                  const struct multiboot_tag_mmap *mmap_tag) {
    kassert(old_acpi_tag->size == 28);

    // Copy RSDP to kernel range because RSDP is outside kernel range and it is
    // considered invalid after pmm_init.
    g_rsdp = *(const struct rsdp *)old_acpi_tag->rsdp;
    kassert(acpi_rsdp_is_valid_checksum(&g_rsdp));
    // Assert that we have an old ACPI RSDP
    kassert(g_rsdp.revision == 0);

    acpi_print_rsdp(&g_rsdp);

    bool acpi_region_found = false;
    for (uint64_t i = 0; sizeof(struct multiboot_tag_mmap)
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

uint64_t acpi_map_region(void) {
    return vmm_map_physical(ACPI_VIRT_ADDR, acpi_region_addr, acpi_region_len,
                            0);
}

const struct description_header *acpi_rsdt_find_table(char signature[4]) {
    const struct rsdt *rsdt = (void *)ACPI_PHYS_TO_VIRT(g_rsdp.rsdt_phys_addr);

    const struct description_header *description_header = NULL;
    for (uint64_t i = 0; i * sizeof(uint32_t) + offsetof(struct rsdt, entry)
         < rsdt->header.length;
         ++i) {
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

bool acpi_rsdp_is_valid_checksum(const struct rsdp *rsdp) {
    uint8_t sum = 0;
    for (uint32_t i = 0; i < sizeof(struct rsdp); ++i) {
        sum += ((const uint8_t *)rsdp)[i];
    }

    return sum == 0;
}

bool acpi_table_is_valid_checksum(const struct description_header *header) {
    uint8_t sum = 0;
    for (uint32_t i = 0; i < header->length; ++i) {
        sum += ((const uint8_t *)header)[i];
    }

    return sum == 0;
}

void acpi_print_rsdp(const struct rsdp *rsdp) {
    kprintf("RSDP:\n");

    kprintf("  Signature: %c%c%c%c%c%c%c%c\n", rsdp->signature[0],
            rsdp->signature[1], rsdp->signature[2], rsdp->signature[3],
            rsdp->signature[4], rsdp->signature[5], rsdp->signature[6],
            rsdp->signature[7]);
    kprintf("  Checksum: %u\n", rsdp->checksum);
    kprintf("  OEM ID: %c%c%c%c%c%c\n", rsdp->oem_id[0], rsdp->oem_id[1],
            rsdp->oem_id[2], rsdp->oem_id[3], rsdp->oem_id[4], rsdp->oem_id[5]);
    kprintf("  Revision: %u\n", rsdp->revision);
    kprintf("  RSDT physical address: %x\n", rsdp->rsdt_phys_addr);
}
