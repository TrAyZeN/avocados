#ifndef AVOCADOS_PAGING_H_
#define AVOCADOS_PAGING_H_

#include <stdbool.h>
#include <stdint.h>

#include "attributes.h"
#include "utils.h"

// Paging structure size is 4096 bytes (see Vol. 3A 4.2)
#define PAGING_STRUCT_SIZE 4096U

struct pml4e {
    uint64_t present : 1;
    uint64_t rw : 1;
    uint64_t us : 1;
    uint64_t pwt : 1;
    uint64_t pcd : 1;
    uint64_t reserved : 6;
    uint64_t r : 1;
    uint64_t addr : 40;
    uint64_t ignored : 11;
    uint64_t xd : 1;
} __packed;
_Static_assert(sizeof(struct pml4e) == 8);
typedef struct pml4e pml4_t[PAGING_STRUCT_SIZE / sizeof(struct pml4e)] __align(
    PAGING_STRUCT_SIZE);

struct pdpte {
    uint64_t present : 1;
    uint64_t rw : 1;
    uint64_t us : 1;
    uint64_t pwt : 1;
    uint64_t pcd : 1;
    uint64_t reserved : 6;
    uint64_t r : 1;
    uint64_t addr : 40;
    uint64_t ignored : 11;
    uint64_t xd : 1;
} __packed;
_Static_assert(sizeof(struct pdpte) == 8);
typedef struct pdpte pdpt_t[PAGING_STRUCT_SIZE / sizeof(struct pdpte)] __align(
    PAGING_STRUCT_SIZE);

// TODO: Update
struct pde {
    uint64_t present : 1;
    uint64_t rw : 1;
    uint64_t us : 1;
    uint64_t pwt : 1;
    uint64_t pcd : 1;
    uint64_t reserved : 6;
    uint64_t r : 1;
    uint64_t addr : 40;
    uint64_t ignored : 11;
    uint64_t xd : 1;
} __packed;
_Static_assert(sizeof(struct pde) == 8);
typedef struct pde
    pdt_t[PAGING_STRUCT_SIZE / sizeof(struct pde)] __align(PAGING_STRUCT_SIZE);

// TODO: Update
struct pte {
    uint64_t present : 1;
    uint64_t rw : 1;
    uint64_t us : 1;
    uint64_t pwt : 1;
    uint64_t pcd : 1;
    uint64_t reserved : 6;
    uint64_t r : 1;
    uint64_t addr : 40;
    uint64_t ignored : 11;
    uint64_t xd : 1;
} __packed;
_Static_assert(sizeof(struct pte) == 8);
typedef struct pte
    pt_t[PAGING_STRUCT_SIZE / sizeof(struct pte)] __align(PAGING_STRUCT_SIZE);

// An address is considered to be in canonical form if address bits 63 through
// the most-significant implemented bit are set to either all ones or all zeros.
// Vol. 1 3.3.7.1
// 4-level paging supports 48-bits virtual addresses, meaning that bits 48
// through 63 should be the same as bit 47.
static inline bool is_canonical(uint64_t virt_addr) {
    uint64_t ext = BIT_RANGE(virt_addr, 47, 63);

    // There are 17 bits in the range 47 to 63
    return ext == 0x1ffff || ext == 0x00000;
}

// Return a pointer to the pml4e of the given virtual address using recursive
// paging.
static inline struct pml4e *get_pml4e(uint64_t virt_addr) {
    return (struct pml4e *)(0xfffffffffffff000
                            | (((virt_addr >> 39) & ((1 << 9) - 1)) * 8));
}

// Return a pointer to the pdpte of the given virtual address using recursive
// paging.
static inline struct pdpte *get_pdpte(uint64_t virt_addr) {
    return (struct pdpte *)(0xffffffffffe00000
                            | (((virt_addr >> 39) & ((1 << 9) - 1)) << 12)
                            | (((virt_addr >> 30) & ((1 << 9) - 1)) * 8));
}

// Return a pointer to the pde of the given virtual address using recursive
// paging.
static inline struct pde *get_pde(uint64_t virt_addr) {
    return (struct pde *)(0xffffffffc0000000
                          | (((virt_addr >> 30) & ((1 << 18) - 1)) << 12)
                          | (((virt_addr >> 21) & ((1 << 9) - 1)) * 8));
}

// Return a pointer to the pte of the given virtual address using recursive
// paging.
static inline struct pte *get_pte(uint64_t virt_addr) {
    return (struct pte *)(0xffffff8000000000
                          | (((virt_addr >> 21) & ((1 << 27) - 1)) << 12)
                          | (((virt_addr >> 12) & ((1 << 9) - 1)) * 8));
}

#endif /* ! AVOCADOS_PAGING_H_ */
