#ifndef AVOCADOS_PMM_H_
#define AVOCADOS_PMM_H_

#include "attributes.h"
#include "multiboot2.h"
#include "types.h"

// Pages in physical memory are called page frames. (Vol. 3A 2.1.5)
// TODO: Assert that page frame size is a power of two
#define PAGE_SIZE 4096U

#define PMM_ALLOC_ERROR 0xffffffffffffffffUL

void pmm_init(const struct multiboot_tag_mmap *mmap_tag);
u64 pmm_alloc(void) __warn_unused_result;
void pmm_free(u64 addr);

#endif /* ! AVOCADOS_PMM_H_ */
