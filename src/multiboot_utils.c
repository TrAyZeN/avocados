#include <stdint.h>

#include "kprintf.h"
#include "multiboot_utils.h"

struct multiboot_tag *multiboot_find_tag(struct multiboot_tag *tag,
                                         multiboot_uint16_t type) {
    while (tag->type != MULTIBOOT_TAG_TYPE_END && tag->type != type) {
        tag = (struct multiboot_tag *)((multiboot_uint8_t *)tag
                                       + ((tag->size + 7) & ~7U));
    }

    return tag;
}

void multiboot_print_mmap(const struct multiboot_tag_mmap *mmap_tag) {
    puts("multiboot mmap:\n");

    for (uint64_t i = 0; sizeof(struct multiboot_tag_mmap)
             + i * sizeof(struct multiboot_mmap_entry)
         < mmap_tag->size;
         i++) {
        const char *type;
        switch (mmap_tag->entries[i].type) {
        case MULTIBOOT_MEMORY_AVAILABLE:
            type = "available";
            break;
        case MULTIBOOT_MEMORY_RESERVED:
            type = "reserved";
            break;
        case MULTIBOOT_MEMORY_ACPI_RECLAIMABLE:
            type = "acpi reclaimable";
            break;
        case MULTIBOOT_MEMORY_NVS:
            type = "nvs";
            break;
        case MULTIBOOT_MEMORY_BADRAM:
            type = "bad ram";
            break;
        default:
            type = "unknown (maybe something went wrong)";
            break;
        }

        kprintf("  %llx %llx %s\n", mmap_tag->entries[i].addr,
                mmap_tag->entries[i].len, type);
    }
}
