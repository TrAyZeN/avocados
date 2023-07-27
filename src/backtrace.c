#include <stdint.h>

#include "kassert.h"
#include "kprintf.h"

extern uint8_t _eh_frame_start;

#define DWARF_CIE_AUG_SIZE (1 << 0)
#define DWARF_CIE_AUG_CODE_ENC (1 << 1)

struct dwarf_cie {
    uint32_t length;
    uint32_t id;
    uint8_t version;
    uint8_t augmentation;
    uint64_t code_alignment_factor;
    int64_t data_alignment_factor;
    uint64_t return_address_register;
    const uint8_t *instructions;
};

struct dwarf_fde {
    uint32_t length;
    const struct dwarf_cie *cie;
};

static uint64_t decode_uleb128(const uint8_t *buf, uint64_t *result) {
    *result = 0;

    uint64_t i = 0;
    uint64_t shift = 0;
    while (((buf[i] >> 8) & 1) == 1) {
        kassert(i * 7 + 7 < 64);

        *result |= (buf[i] & 0x7fUL) << shift;

        shift += 7;
        i += 1;
    }

    // Last byte
    kassert(i * 7 + 7 < 64);
    *result |= (buf[i] & 0x7fUL) << shift;
    i += 1;

    return i;
}

static uint64_t decode_sleb128(const uint8_t *buf, int64_t *result) {
    *result = 0;

    uint64_t i = 0;
    uint64_t shift = 0;
    while (((buf[i] >> 8) & 1) == 1) {
        kassert(i * 7 + 7 < 64);

        *result |= (buf[i] & 0x7f) << shift;

        shift += 7;
        i += 1;
    }

    // Last byte
    kassert(i * 7 + 7 < 64);
    *result |= (buf[i] & 0x7f) << shift;
    shift += 7;
    i += 1;

    // Sign extend negative
    if (shift < 64 && (*result & (1 << (shift - 1))) != 0) {
        *result = (int64_t)(*(uint64_t *)result | (~0UL << shift));
    }

    return i;
}

static uint64_t dwarf_parse_cie(const uint8_t *buf, struct dwarf_cie *cie) {
    cie->length = *(uint32_t *)buf;
    cie->id = *(uint32_t *)(buf + 4);
    kassert(cie->id == 0);

    cie->version = buf[8];

    uint64_t offset = 9;
    cie->augmentation = 0;
    while (buf[offset] != '\0') {
        switch (buf[offset]) {
        case 'z':
            cie->augmentation |= DWARF_CIE_AUG_SIZE;
            break;
        case 'R':
            cie->augmentation |= DWARF_CIE_AUG_CODE_ENC;
            break;
        default:
            kpanic("Unhandled augmentation char %c\n", buf[offset]);
            break;
        }

        offset += 1;
    }

    // Skip augmentation string null terminator
    offset += 1;

    offset += decode_uleb128(buf + offset, &cie->code_alignment_factor);
    offset += decode_sleb128(buf + offset, &cie->data_alignment_factor);
    offset += decode_uleb128(buf + offset, &cie->return_address_register);

    cie->instructions = buf + offset;

    return cie->length + 4;
}

static void dwarf_print_cie(const struct dwarf_cie *cie) {
    kprintf("CIE:\n");
    kprintf("  Length: %u\n", cie->length);
    kprintf("  Id: %u\n", cie->id);
    kprintf("  Version: %u\n", cie->version);
    kprintf("  Augmentation: %u\n", cie->augmentation);
    kprintf("  Code alignment factor: %lu\n", cie->code_alignment_factor);
    kprintf("  Data alignment factor: %ld\n", cie->data_alignment_factor);
    kprintf("  Return address register: %lu\n", cie->return_address_register);
}

static uint64_t dwarf_parse_fde(const uint8_t *buf, struct dwarf_fde *fde,
                                const struct dwarf_cie *cie) {
    fde->length = *(uint32_t *)buf;
    // We make the assumption that there is only one CIE
    fde->cie = cie;
    kassert(*(uint32_t *)(buf + 4) != 0);

    // TODO: Initial location

    // https://refspecs.linuxfoundation.org/LSB_4.1.0/LSB-Core-generic/LSB-Core-generic/ehframechpt.html

    return fde->length + 4;
}

static void dwarf_print_fde(const struct dwarf_fde *fde) {
    kprintf("FDE:\n");
    kprintf("  Length: %u\n", fde->length);
}

// TODO: Check .eh_frame's DWARF version
// return_address_register type depends on DWARF version
void backtrace(void) {
    /* kprintf("_eh_frame_start: %lx\n", (uint64_t)&_eh_frame_start); */
    /* for (uint64_t i = 0; i < 20; ++i) { */
    /* uint8_t byte = ((uint8_t *)&_eh_frame_start)[i]; */
    /* kprintf("%x ", byte); */
    /* } */
    /* puts("\n"); */

    struct dwarf_cie cie;
    struct dwarf_fde fde;

    const uint8_t *eh_frame = &_eh_frame_start;

    uint64_t offset = 0;
    offset += dwarf_parse_cie(eh_frame, &cie);
    dwarf_print_cie(&cie);

    kprintf("offset: %lu\n", offset);
    offset += dwarf_parse_fde(eh_frame + offset, &fde, &cie);
    dwarf_print_fde(&fde);
}
