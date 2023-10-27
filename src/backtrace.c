#include "kassert.h"
#include "kprintf.h"
#include "types.h"

extern u8 _eh_frame_start;

#define DWARF_CIE_AUG_SIZE (1 << 0)
#define DWARF_CIE_AUG_CODE_ENC (1 << 1)

typedef struct {
    u32 length;
    u32 id;
    u8 version;
    u8 augmentation;
    u64 code_alignment_factor;
    i64 data_alignment_factor;
    u64 return_address_register;
    const u8 *instructions;
} dwarf_cie_t;

typedef struct {
    u32 length;
    const dwarf_cie_t *cie;
} dwarf_fde_t;

static u64 decode_uleb128(const u8 *buf, u64 *result) {
    *result = 0;

    u64 i = 0;
    u64 shift = 0;
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

static u64 decode_sleb128(const u8 *buf, i64 *result) {
    *result = 0;

    u64 i = 0;
    u64 shift = 0;
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
        *result = (i64)(*(u64 *)result | (~0UL << shift));
    }

    return i;
}

static u64 dwarf_parse_cie(const u8 *buf, dwarf_cie_t *cie) {
    cie->length = *(u32 *)buf;
    cie->id = *(u32 *)(buf + 4);
    kassert(cie->id == 0);

    cie->version = buf[8];

    u64 offset = 9;
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

static void dwarf_print_cie(const dwarf_cie_t *cie) {
    kprintf("CIE:\n");
    kprintf("  Length: %u\n", cie->length);
    kprintf("  Id: %u\n", cie->id);
    kprintf("  Version: %u\n", cie->version);
    kprintf("  Augmentation: %u\n", cie->augmentation);
    kprintf("  Code alignment factor: %lu\n", cie->code_alignment_factor);
    kprintf("  Data alignment factor: %ld\n", cie->data_alignment_factor);
    kprintf("  Return address register: %lu\n", cie->return_address_register);
}

static u64 dwarf_parse_fde(const u8 *buf, dwarf_fde_t *fde,
                           const dwarf_cie_t *cie) {
    fde->length = *(u32 *)buf;
    // We make the assumption that there is only one CIE
    fde->cie = cie;
    kassert(*(u32 *)(buf + 4) != 0);

    // TODO: Initial location

    // https://refspecs.linuxfoundation.org/LSB_4.1.0/LSB-Core-generic/LSB-Core-generic/ehframechpt.html

    return fde->length + 4;
}

static void dwarf_print_fde(const dwarf_fde_t *fde) {
    kprintf("FDE:\n");
    kprintf("  Length: %u\n", fde->length);
}

// TODO: Check .eh_frame's DWARF version
// return_address_register type depends on DWARF version
void backtrace(void) {
    /* kprintf("_eh_frame_start: %lx\n", (u64)&_eh_frame_start); */
    /* for (u64 i = 0; i < 20; ++i) { */
    /* u8 byte = ((u8 *)&_eh_frame_start)[i]; */
    /* kprintf("%x ", byte); */
    /* } */
    /* puts("\n"); */

    dwarf_cie_t cie;
    dwarf_fde_t fde;

    const u8 *eh_frame = &_eh_frame_start;

    u64 offset = 0;
    offset += dwarf_parse_cie(eh_frame, &cie);
    dwarf_print_cie(&cie);

    kprintf("offset: %lu\n", offset);
    offset += dwarf_parse_fde(eh_frame + offset, &fde, &cie);
    dwarf_print_fde(&fde);
}
