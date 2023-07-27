#ifndef AVOCADOS_VMM_H_
#define AVOCADOS_VMM_H_

#include <stdint.h>

#include "attributes.h"

#define VMM_ALLOC_ERROR 0xffffffffffffffffUL

#define VMM_ALLOC_RW (1 << 0)

void vmm_init(void);
uint64_t vmm_alloc(uint64_t addr, uint32_t flags) __warn_unused_result;
void vmm_free(uint64_t addr);

uint64_t vmm_map_physical(uint64_t virt_addr, uint64_t phys_addr, uint64_t len,
                          uint32_t flags) __warn_unused_result;

#endif /* ! AVOCADOS_VMM_H_ */
