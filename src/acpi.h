#ifndef AVOCADOS_ACPI_H_
#define AVOCADOS_ACPI_H_

#include <stdbool.h>
#include <stdint.h>

#include "attributes.h"
#include "multiboot2.h"

// Virtual address of ACPI memory mapping
#define ACPI_VIRT_ADDR 0x0000001000000000UL

// Physical address of ACPI region
extern uint64_t acpi_region_addr;
#define ACPI_PHYS_TO_VIRT(PHYS) ((PHYS)-acpi_region_addr + ACPI_VIRT_ADDR)

struct rsdp {
    char signature[8];
    uint8_t checksum;
    char oem_id[6];
    uint8_t revision;
    uint32_t rsdt_phys_addr;
} __packed;

struct description_header {
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oem_id[6];
    char oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} __packed;

struct rsdt {
    struct description_header header;
    uint32_t entry[];
} __packed;

struct madt_int_controller {
#define MADT_INT_CONTROLLER_LAPIC 0
#define MADT_INT_CONTROLLER_IOAPIC 1
#define MADT_INT_CONTROLLER_INT_SRC_OVERRIDE 2
#define MADT_INT_CONTROLLER_NMI_SRC 3
#define MADT_INT_CONTROLLER_LAPIC_NMI 4
#define MADT_INT_CONTROLLER_LAPIC_ADDR_OVERRIDE 5
#define MADT_INT_CONTROLLER_X2APIC 9
#define MADT_INT_CONTROLLER_X2APIC_NMI 0xa
    uint8_t type;
} __packed;

struct madt {
    struct description_header header;
    uint32_t lapic_phys_addr;
    uint32_t flags;
    struct madt_int_controller int_controllers;
} __packed;

struct madt_int_controller_lapic {
    uint8_t type;
    uint8_t length;
    uint8_t acpi_processor_id;
    uint8_t apic_id;
    uint32_t flags;
} __packed;

struct madt_int_controller_ioapic {
    uint8_t type;
    uint8_t length;
    uint8_t io_apic_id;
    uint8_t reserved;
    uint32_t io_apic_phys_addr;
    uint32_t system_vector_base;
} __packed;

void acpi_prepare(const struct multiboot_tag_old_acpi *old_acpi_tag,
                  const struct multiboot_tag_mmap *mmap_tag);
uint64_t acpi_map_region(void);

const struct description_header *acpi_rsdt_find_table(char signature[4]);

bool acpi_rsdp_is_valid_checksum(const struct rsdp *rsdp);
bool acpi_table_is_valid_checksum(const struct description_header *header);

void acpi_print_rsdp(const struct rsdp *rsdp);

#endif /* ! AVOCADOS_ACPI_H_ */
