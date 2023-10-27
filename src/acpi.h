#ifndef AVOCADOS_ACPI_H_
#define AVOCADOS_ACPI_H_

#include <stdbool.h>

#include "attributes.h"
#include "multiboot2.h"
#include "types.h"

// Virtual address of ACPI memory mapping
#define ACPI_VIRT_ADDR 0x0000001000000000UL

// Physical address of ACPI region
extern u64 acpi_region_addr;
#define ACPI_PHYS_TO_VIRT(PHYS) ((PHYS)-acpi_region_addr + ACPI_VIRT_ADDR)

typedef struct {
    char signature[8];
    u8 checksum;
    char oem_id[6];
    u8 revision;
    u32 rsdt_phys_addr;
} __packed rsdp_t;

typedef struct {
    char signature[4];
    u32 length;
    u8 revision;
    u8 checksum;
    char oem_id[6];
    char oem_table_id[8];
    u32 oem_revision;
    u32 creator_id;
    u32 creator_revision;
} __packed description_header_t;

typedef struct {
    description_header_t header;
    u32 entry[];
} __packed rsdt_t;

typedef struct {
#define MADT_INT_CONTROLLER_LAPIC 0
#define MADT_INT_CONTROLLER_IOAPIC 1
#define MADT_INT_CONTROLLER_INT_SRC_OVERRIDE 2
#define MADT_INT_CONTROLLER_NMI_SRC 3
#define MADT_INT_CONTROLLER_LAPIC_NMI 4
#define MADT_INT_CONTROLLER_LAPIC_ADDR_OVERRIDE 5
#define MADT_INT_CONTROLLER_X2APIC 9
#define MADT_INT_CONTROLLER_X2APIC_NMI 0xa
    u8 type;
} __packed madt_int_controller_t;

typedef struct {
    description_header_t header;
    u32 lapic_phys_addr;
    u32 flags;
    madt_int_controller_t int_controllers;
} __packed madt_t;

typedef struct {
    u8 type;
    u8 length;
    u8 acpi_processor_id;
    u8 apic_id;
    u32 flags;
} __packed madt_int_controller_lapic_t;

typedef struct {
    u8 type;
    u8 length;
    u8 ioapic_id;
    u8 __reserved;
    u32 ioapic_phys_addr;
    u32 system_vector_base;
} __packed madt_int_controller_ioapic_t;

typedef struct {
    u8 type;
    u8 length;
    u8 bus;
    u8 source;
    u32 global_system_interrupt;
    u16 flags;
} __packed madt_int_controller_int_src_override_t;

typedef struct {
    u8 type;
    u8 length;
    u8 acpi_processor_id;
    u16 flags;
    u8 lapic_lint_num;
} __packed madt_int_controller_lapic_nmi_t;

typedef struct {
    u8 address_space_id;
    u8 reg_bit_width;
    u8 reg_bit_offset;
    u8 access_size;
    u64 addr;
} __packed generic_address_structure_t;

typedef struct {
    description_header_t header;
    u32 event_timer_block_id;
    generic_address_structure_t base_address;
    u8 hpet_num;
    u16 min_ticks;
    u8 attrs;
} __packed hpet_description_table_t;

void acpi_prepare(const struct multiboot_tag_old_acpi *old_acpi_tag,
                  const struct multiboot_tag_mmap *mmap_tag);
u64 acpi_map_region(void);

const description_header_t *acpi_rsdt_find_table(char signature[4]);
u32 acpi_madt_find_ioapic_addr(const madt_t *madt);

bool acpi_rsdp_is_valid_checksum(const rsdp_t *rsdp);
bool acpi_table_is_valid_checksum(const description_header_t *header);

void acpi_print_rsdp(const rsdp_t *rsdp);
void acpi_print_madt(const madt_t *madt);
void acpi_print_hpet_description_table(const hpet_description_table_t *hdt);

#endif /* ! AVOCADOS_ACPI_H_ */
