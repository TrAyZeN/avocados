#include "acpi.h"
#include "kprintf.h"

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
