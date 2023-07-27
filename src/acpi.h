#ifndef AVOCADOS_ACPI_H_
#define AVOCADOS_ACPI_H_

#include <stdint.h>

#include "attributes.h"

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

void acpi_print_rsdp(const struct rsdp *rsdp);

#endif /* ! AVOCADOS_ACPI_H_ */
