#ifndef AVOCADOS_PAGING_H_
#define AVOCADOS_PAGING_H_

#include <stdbool.h>

#include "attributes.h"
#include "libk/kassert.h"
#include "types.h"
#include "utils.h"

// Paging structure size is 4096 bytes (see Vol. 3A 4.2)
#define PAGING_STRUCT_SIZE 4096U

// PML4 entry that references a page-directory-pointer table
typedef struct {
    u64 present : 1;
    u64 rw : 1;
    u64 us : 1;
    u64 pwt : 1;
    u64 pcd : 1;
    u64 reserved : 6;
    u64 r : 1;
    u64 addr : 40;
    u64 ignored : 11;
    u64 xd : 1;
} __packed pml4e_t;
_Static_assert(sizeof(pml4e_t) == 8);
typedef pml4e_t
    pml4_t[PAGING_STRUCT_SIZE / sizeof(pml4e_t)] __align(PAGING_STRUCT_SIZE);

// Page-directory-pointer-table entry that references a page directory
typedef struct {
    u64 present : 1;
    u64 rw : 1;
    u64 us : 1;
    u64 pwt : 1;
    u64 pcd : 1;
    u64 reserved : 6;
    u64 r : 1;
    u64 addr : 40;
    u64 ignored : 11;
    u64 xd : 1;
} __packed pdpte_t;
_Static_assert(sizeof(pdpte_t) == 8);
typedef pdpte_t
    pdpt_t[PAGING_STRUCT_SIZE / sizeof(pdpte_t)] __align(PAGING_STRUCT_SIZE);

// Page-directory entry that references a page table
typedef struct {
    u64 present : 1;
    u64 rw : 1;
    u64 us : 1;
    u64 pwt : 1;
    u64 pcd : 1;
    u64 reserved : 6;
    u64 r : 1;
    u64 addr : 40;
    u64 ignored : 11;
    u64 xd : 1;
} __packed pde_t;
_Static_assert(sizeof(pde_t) == 8);
typedef pde_t
    pdt_t[PAGING_STRUCT_SIZE / sizeof(pde_t)] __align(PAGING_STRUCT_SIZE);

// Page-table entry that maps a 4-KByte page
typedef struct {
    u64 present : 1;
    u64 rw : 1;
    u64 us : 1;
    u64 pwt : 1;
    u64 pcd : 1;
    u64 reserved : 6;
    u64 r : 1;
    u64 addr : 40;
    u64 ignored : 11;
    u64 xd : 1;
} __packed pte_t;
_Static_assert(sizeof(pte_t) == 8);
typedef pte_t
    pt_t[PAGING_STRUCT_SIZE / sizeof(pte_t)] __align(PAGING_STRUCT_SIZE);

// An address is considered to be in canonical form if address bits 63 through
// the most-significant implemented bit are set to either all ones or all zeros.
// Vol. 1 3.3.7.1
// 4-level paging supports 48-bits virtual addresses, meaning that bits 48
// through 63 should be the same as bit 47.
static inline bool is_canonical(u64 virt_addr) {
    u64 ext = BIT_RANGE(virt_addr, 47, 63);

    // There are 17 bits in the range 47 to 63
    return ext == 0x1ffff || ext == 0x00000;
}

// Return a pointer to the pml4e of the given virtual address using recursive
// paging.
static inline pml4e_t *get_pml4e(u64 virt_addr) {
    kassert(is_canonical(virt_addr));

    return (pml4e_t *)(0xfffffffffffff000
                       | (((virt_addr >> 39) & ((1 << 9) - 1)) * 8));
}

// Return a pointer to the pdpte of the given virtual address using recursive
// paging.
static inline pdpte_t *get_pdpte(u64 virt_addr) {
    kassert(is_canonical(virt_addr));

    return (pdpte_t *)(0xffffffffffe00000
                       | (((virt_addr >> 39) & ((1 << 9) - 1)) << 12)
                       | (((virt_addr >> 30) & ((1 << 9) - 1)) * 8));
}

// Return a pointer to the pde of the given virtual address using recursive
// paging.
static inline pde_t *get_pde(u64 virt_addr) {
    kassert(is_canonical(virt_addr));

    return (pde_t *)(0xffffffffc0000000
                     | (((virt_addr >> 30) & ((1 << 18) - 1)) << 12)
                     | (((virt_addr >> 21) & ((1 << 9) - 1)) * 8));
}

// Return a pointer to the pte of the given virtual address using recursive
// paging.
static inline pte_t *get_pte(u64 virt_addr) {
    kassert(is_canonical(virt_addr));

    return (pte_t *)(0xffffff8000000000
                     | (((virt_addr >> 21) & ((1 << 27) - 1)) << 12)
                     | (((virt_addr >> 12) & ((1 << 9) - 1)) * 8));
}

#endif /* ! AVOCADOS_PAGING_H_ */
