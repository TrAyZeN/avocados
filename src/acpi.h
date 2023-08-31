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
    uint8_t ioapic_id;
    uint8_t __reserved;
    uint32_t ioapic_phys_addr;
    uint32_t system_vector_base;
} __packed;

struct madt_int_controller_int_src_override {
    uint8_t type;
    uint8_t length;
    uint8_t bus;
    uint8_t source;
    uint32_t global_system_interrupt;
    uint16_t flags;
} __packed;

struct madt_int_controller_lapic_nmi {
    uint8_t type;
    uint8_t length;
    uint8_t acpi_processor_id;
    uint16_t flags;
    uint8_t lapic_lint_num;
} __packed;

struct generic_address_structure {
    uint8_t address_space_id;
    uint8_t reg_bit_width;
    uint8_t reg_bit_offset;
    uint8_t access_size;
    uint64_t addr;
} __packed;

struct hpet_description_table {
    struct description_header header;
    uint32_t event_timer_block_id;
    struct generic_address_structure base_address;
    uint8_t hpet_num;
    uint16_t min_ticks;
    uint8_t attrs;
} __packed;

void acpi_prepare(const struct multiboot_tag_old_acpi *old_acpi_tag,
                  const struct multiboot_tag_mmap *mmap_tag);
uint64_t acpi_map_region(void);

const struct description_header *acpi_rsdt_find_table(char signature[4]);
uint32_t acpi_madt_find_ioapic_addr(const struct madt *madt);

bool acpi_rsdp_is_valid_checksum(const struct rsdp *rsdp);
bool acpi_table_is_valid_checksum(const struct description_header *header);

void acpi_print_rsdp(const struct rsdp *rsdp);
void acpi_print_madt(const struct madt *madt);
void acpi_print_hpet_description_table(
    const struct hpet_description_table *hdt);

#endif /* ! AVOCADOS_ACPI_H_ */
