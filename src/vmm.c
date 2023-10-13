#include "kassert.h"
#include "log.h"
#include "paging.h"
#include "pmm.h"
#include "vmm.h"

static u64 vmm_alloc_paging_structs(u64 addr);

void vmm_init(void) {
    extern u8 _srodata, _erodata;
    extern u8 _eh_frame_start, _eh_frame_end;
    extern u8 _sdata, _edata;
    extern u8 _sbss, _ebss;

    for (u64 addr = (u64)&_srodata; addr < (u64)&_erodata; addr += PAGE_SIZE) {
        struct pte *pte = get_pte(addr);

        pte->rw = 0;
        pte->xd = 1;
    }

    for (u64 addr = (u64)&_eh_frame_start; addr < (u64)&_eh_frame_end;
         addr += PAGE_SIZE) {
        struct pte *pte = get_pte(addr);

        pte->rw = 0;
        pte->xd = 1;
    }

    for (u64 addr = (u64)&_eh_frame_start; addr < (u64)&_eh_frame_end;
         addr += PAGE_SIZE) {
        struct pte *pte = get_pte(addr);

        pte->rw = 0;
        pte->xd = 1;
    }

    for (u64 addr = (u64)&_sdata; addr < (u64)&_edata; addr += PAGE_SIZE) {
        struct pte *pte = get_pte(addr);

        pte->xd = 1;
    }

    for (u64 addr = (u64)&_sbss; addr < (u64)&_ebss; addr += PAGE_SIZE) {
        struct pte *pte = get_pte(addr);

        pte->xd = 1;
    }

    log(LOG_LEVEL_INFO, "VMM: VMM initialized\n");
}

u64 vmm_alloc(u64 addr, u32 flags) {
    kassert(addr % PAGE_SIZE == 0);
    kassert(is_canonical(addr));

    u64 res = vmm_alloc_paging_structs(addr);
    if (res == PMM_ALLOC_ERROR) {
        goto failed_paging_structs_alloc;
    }

    struct pte *pte = get_pte(addr);
    kassert(!pte->present);

    u64 page_phys_addr = pmm_alloc();
    if (page_phys_addr == PMM_ALLOC_ERROR) {
        goto failed_page_alloc;
    }

    *pte = (struct pte){
        .present = 1,
        .rw = (flags & VMM_ALLOC_RW) ? 1U : 0U,
        .us = (flags & VMM_ALLOC_USER) ? 1U : 0U,
        .addr = BIT_RANGE(page_phys_addr, 12, 51),
        .xd = ((flags & VMM_ALLOC_EXEC) ? 0U : 1U) & 1,
    };
    memset((u8 *)addr, 0, PAGE_SIZE);

    return addr;

failed_paging_structs_alloc:
failed_page_alloc:
    log(LOG_LEVEL_WARN, "VMM: PMM allocation failed\n");
    return VMM_ALLOC_ERROR;
}

// Free a single page
void vmm_free(u64 addr) {
    kassert(addr % PAGE_SIZE == 0);
    kassert(is_canonical(addr));

    struct pte *pte = get_pte(addr);
    pte->present = 0;
    pmm_free(pte->addr << 12);
}

u64 vmm_map_physical(u64 virt_addr, u64 phys_addr, u64 len, u32 flags) {
    kassert(virt_addr % PAGE_SIZE == 0);
    kassert(is_canonical(virt_addr));
    kassert(phys_addr % PAGE_SIZE == 0);
    kassert(len % PAGE_SIZE == 0);

    for (u64 offset = 0; offset < len; offset += PAGE_SIZE) {
        u64 res = vmm_alloc_paging_structs(virt_addr + offset);
        if (res == VMM_ALLOC_ERROR) {
            goto failed_paging_structs_alloc;
        }

        struct pte *pte = get_pte(virt_addr + offset);
        kassert(!pte->present);

        *pte = (struct pte){
            .present = 1,
            .rw = (flags & VMM_ALLOC_RW) ? 1U : 0U,
            .us = (flags & VMM_ALLOC_USER) ? 1U : 0U,
            .addr = BIT_RANGE(phys_addr + offset, 12, 51),
            .xd = ((flags & VMM_ALLOC_EXEC) ? 0U : 1U) & 1,
        };
    }

    return 0;

failed_paging_structs_alloc:
    return VMM_ALLOC_ERROR;
}

static u64 vmm_alloc_paging_structs(u64 addr) {
    kassert(addr % PAGE_SIZE == 0);
    kassert(is_canonical(addr));

    // TODO: Handle case when pml4e not present but pdpte present

    struct pml4e *pml4e = get_pml4e(addr);
    if (!pml4e->present) {
        log(LOG_LEVEL_DEBUG, "VMM: pml4e not present\n");

        u64 pdpt_phys_addr = pmm_alloc();
        if (pdpt_phys_addr == PMM_ALLOC_ERROR) {
            goto failed_pdpt_alloc;
        }

        *pml4e = (struct pml4e){
            .present = 1,
            .rw = 1,
            .us = 0,
            .addr = BIT_RANGE(pdpt_phys_addr, 12, 51),
            .xd = 0,
        };

        // Zero initialize the table so that present bits are 0
        memset((u8 *)ALIGN_DOWN((u64)get_pdpte(addr), PAGE_SIZE), 0, PAGE_SIZE);
    }

    struct pdpte *pdpte = get_pdpte(addr);
    if (!pdpte->present) {
        log(LOG_LEVEL_DEBUG, "VMM: pdpte not present\n");

        u64 pdt_phys_addr = pmm_alloc();
        if (pdt_phys_addr == PMM_ALLOC_ERROR) {
            goto failed_pdt_alloc;
        }

        *pdpte = (struct pdpte){
            .present = 1,
            .rw = 1,
            .us = 0,
            .addr = BIT_RANGE(pdt_phys_addr, 12, 51),
            .xd = 0,
        };

        // Zero initialize the table so that present bits are 0
        memset((u8 *)ALIGN_DOWN((u64)get_pde(addr), PAGE_SIZE), 0, PAGE_SIZE);
    }

    struct pde *pde = get_pde(addr);
    if (!pde->present) {
        log(LOG_LEVEL_DEBUG, "VMM: pde not present\n");

        u64 pt_phys_addr = pmm_alloc();
        if (pt_phys_addr == PMM_ALLOC_ERROR) {
            goto failed_pt_alloc;
        }

        *pde = (struct pde){
            .present = 1,
            .rw = 1,
            .us = 0,
            .addr = BIT_RANGE(pt_phys_addr, 12, 51),
            .xd = 0,
        };

        // Zero initialize the table so that present bits are 0
        memset((u8 *)ALIGN_DOWN((u64)get_pte(addr), PAGE_SIZE), 0, PAGE_SIZE);
    }

    return 0;

failed_pt_alloc:
failed_pdt_alloc:
failed_pdpt_alloc:
    return VMM_ALLOC_ERROR;
}
