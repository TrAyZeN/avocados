#ifndef AVOCADOS_VMM_H_
#define AVOCADOS_VMM_H_

#include "attributes.h"
#include "types.h"

#define VMM_ALLOC_ERROR 0xffffffffffffffffUL

#define VMM_ALLOC_RW (1 << 0)
#define VMM_ALLOC_EXEC (1 << 1)
#define VMM_ALLOC_USER (1 << 2)

void vmm_init(void);
u64 vmm_alloc(u64 addr, u32 flags) __warn_unused_result;
void vmm_free(u64 addr);

u64 vmm_map_physical(u64 virt_addr, u64 phys_addr, u64 len,
                     u32 flags) __warn_unused_result;

#endif /* ! AVOCADOS_VMM_H_ */
