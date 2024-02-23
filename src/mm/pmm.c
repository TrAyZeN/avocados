#include <stddef.h>

#include "arch/paging.h"
#include "libk/bitmap.h"
#include "libk/kassert.h"
#include "libk/kprintf.h"
#include "libk/list.h"
#include "libk/log.h"
#include "libk/mem.h"
#include "libk/string.h"
#include "page_frame_cache.h"
#include "pmm.h"
#include "types.h"
#include "utils.h"

extern u64 _skern;
extern u64 _ekern;

extern pml4_t pml4;

typedef struct memory_map {
    struct memory_map *next;
    // Base physical address of the memory region
    u64 base_addr;
    // Length in bytes of the memory region
    u64 len;
    page_frame_cache_t cache;
    // Page frame allocation state of the memory region. Each bit corresponds to
    // a page frame with 0 = free and 1 = allocated.
    bitmap_t bitmap;
} memory_map_t;

static void memory_map_reserve(memory_map_t *memory_map, u64 base,
                               u64 num_frames);
static void memory_map_reserve_range(memory_map_t *memory_map, u64 base,
                                     u64 end);

static memory_map_t *memory_map = NULL;

#define MAX_MMAP_ENTRIES 20
// Let's map memory map at 0x0000 0000 1000 0000
#define MEMORY_MAP_ADDR 0x0000000010000000

/*
 * Physical memory outside of kernel range (&_skern to &_ekern) is
 * considered free. So beware to copy boot information into kernel range
 * before calling this function. Let's put each bitmap in its region... So
 * that we can be sure it is able to store it
 */
void pmm_init(const struct multiboot_tag_mmap *mmap_tag) {
    kassert(memory_map == NULL);

    log(LOG_LEVEL_WARN,
        "PMM: Physical memory outside 0x%016lx-0x%016lx will be considered "
        "free\n",
        (u64)&_skern, (u64)&_ekern);

    // Copy available memory map entries on the stack (in kernel range)
    u32 num_available_mmap_entries = 0;
    struct multiboot_mmap_entry available_mmap_entries[MAX_MMAP_ENTRIES] = {
        0
    };
    for (u64 i = 0; sizeof(struct multiboot_tag_mmap)
                 + i * sizeof(struct multiboot_mmap_entry)
             < mmap_tag->size
         && num_available_mmap_entries < MAX_MMAP_ENTRIES;
         i++) {
        if (mmap_tag->entries[i].type == MULTIBOOT_MEMORY_AVAILABLE) {
            available_mmap_entries[num_available_mmap_entries] =
                mmap_tag->entries[i];
            num_available_mmap_entries += 1;
        }
    }
    if (num_available_mmap_entries >= MAX_MMAP_ENTRIES) {
        kpanic("Cannot store every available memory entries on the stack");
    }

    u8 first = 1;
    // WARN: Must be a multiple of PAGE_SIZE
    u64 memory_map_virt_addr = MEMORY_MAP_ADDR;
    memory_map_t *prev_memory_map = NULL;
    memory_map_t *curr_memory_map = NULL;
    for (u64 i = 0; i < num_available_mmap_entries; i++) {
        // TODO: Function
        // Compute memory_map_t size with flexible array member
        u64 memory_map_size = sizeof(memory_map_t)
            + ALIGN_UP(available_mmap_entries[i].len,
                       PAGE_SIZE * BITMAP_CHUNK_BITS)
                / (PAGE_SIZE * 8);

        // TODO: Improve first check
        // WARN: Maybe _kern is not the first memory region
        u64 offset = 0;
        if (curr_memory_map == NULL) {
            // Store the memory map after the kernel range
            offset = ALIGN_UP((u64)&_ekern, PAGE_SIZE);
        }
        u64 memory_map_phys_addr =
            ALIGN_UP(available_mmap_entries[i].addr + offset, PAGE_SIZE);

        // Setup paging structures for memory map
        pml4e_t *pml4e = get_pml4e(memory_map_virt_addr);
        if (!pml4e->present) {
            // TODO: Map
            kpanic("Not implemented: pml4e not present\n");
        }

        pdpte_t *pdpte = get_pdpte(memory_map_virt_addr);
        if (!pdpte->present) {
            // TODO: Map
            kpanic("Not implemented: pdpte not present\n");
        }

        u64 res = 0;
        pde_t *pde = get_pde(memory_map_virt_addr);
        if (!pde->present) {
            // TODO: Remove res
            // Store the page table after the memory map
            u64 phys_addr =
                ALIGN_UP(memory_map_phys_addr + memory_map_size, PAGE_SIZE)
                + res * PAGE_SIZE;
            *pde = (pde_t){
                .present = 1,
                .rw = 1,
                .us = 0,
                .addr = BIT_RANGE(phys_addr, 12, 51),
                .xd = 0,
            };

            // Zero initialize the table so that present bits are 0
            memset(
                (u8 *)ALIGN_DOWN((u64)get_pte(memory_map_virt_addr), PAGE_SIZE),
                0, PAGE_SIZE);
            res += 1;
        }

        // Assert that we only need one pde
        kassert(memory_map_virt_addr / (PAGE_SIZE * 512)
                == (memory_map_virt_addr + memory_map_size)
                    / (PAGE_SIZE * 512));

        for (u64 k = 0; k < ALIGN_UP(memory_map_size, PAGE_SIZE);
             k += PAGE_SIZE) {
            pte_t *pte = get_pte(memory_map_virt_addr + k);

            kassert(!pte->present);
            u64 phys_addr = memory_map_phys_addr + k;
            *pte = (pte_t){
                .present = 1,
                .rw = 1,
                .us = 0,
                .addr = BIT_RANGE(phys_addr, 12, 51),
                .xd = 1,
            };
        }

        curr_memory_map = (memory_map_t *)memory_map_virt_addr;
        curr_memory_map->next = NULL;
        curr_memory_map->base_addr = available_mmap_entries[i].addr;
        curr_memory_map->len =
            ALIGN_DOWN(available_mmap_entries[i].len, PAGE_SIZE);
        curr_memory_map->cache.count = 0;
        curr_memory_map->bitmap.size = curr_memory_map->len / PAGE_SIZE;
        memset((u8 *)&curr_memory_map->bitmap.chunks, 0,
               ALIGN_UP(curr_memory_map->bitmap.size, BITS_PER_BYTE)
                   / BITS_PER_BYTE);

        kassert(curr_memory_map->len % PAGE_SIZE == 0);
        kassert(curr_memory_map->base_addr % PAGE_SIZE == 0);
        // Assert that curr_memory_map->base_addr + curr_memory_map->len must
        // not overflow
        kassert(curr_memory_map->len
                <= UINT64_MAX - curr_memory_map->base_addr);

        if (memory_map == NULL) {
            memory_map = curr_memory_map;

            memory_map_reserve_range(curr_memory_map,
                                     ALIGN_DOWN((u64)&_skern, PAGE_SIZE),
                                     ALIGN_UP((u64)&_ekern, PAGE_SIZE));
        } else {
            prev_memory_map->next = curr_memory_map;
        }

        // Reserve memory map pages + page table pages (see res)
        memory_map_reserve_range(
            curr_memory_map, memory_map_phys_addr,
            ALIGN_UP(memory_map_phys_addr + memory_map_size, PAGE_SIZE)
                + res * PAGE_SIZE);

        // Bitmap are u64 but the memory region length may not be a
        // multiple of PAGE_SIZE * 64, so we need to mark these frames as
        // allocated.
        for (u64 frame_idx = curr_memory_map->bitmap.size; frame_idx
             < ALIGN_UP(curr_memory_map->bitmap.size, BITMAP_CHUNK_BITS);
             ++frame_idx) {
            curr_memory_map->bitmap.chunks[frame_idx / BITMAP_CHUNK_BITS] |= 1UL
                << (frame_idx % BITMAP_CHUNK_BITS);
        }

        // TODO: Remove that
        if (first) {
            kprintf("0x%016lx\n", curr_memory_map->bitmap.chunks[0]);
        }

        prev_memory_map = curr_memory_map;
        memory_map_virt_addr += ALIGN_UP(memory_map_size, PAGE_SIZE);
        first = 0;
    }

    log(LOG_LEVEL_INFO, "PMM: PMM initialized\n");
}

// It is assumed that the range is unallocated
// base is a physical address
static void memory_map_reserve(memory_map_t *m, u64 base, u64 num_frames) {
    // Check alignment
    kassert(base % PAGE_SIZE == 0);
    // Check range
    kassert(base >= m->base_addr
            && base + num_frames * PAGE_SIZE < m->base_addr + m->len);

    bitmap_set_range(&m->bitmap, (base - m->base_addr) / PAGE_SIZE, num_frames);
}

// end excluded
static void memory_map_reserve_range(memory_map_t *m, u64 base, u64 end) {
    // Check alignment
    kassert(end % PAGE_SIZE == 0);
    // Check range
    kassert(base < end);

    return memory_map_reserve(m, base, (end - base) / PAGE_SIZE);
}

// Return the physical address of the allocated frame.
// If unable to find a free frame, returns PMM_ALLOC_ERROR.
u64 pmm_alloc(void) {
    u64 phys_addr = PMM_ALLOC_ERROR;

    list_for_each(m, memory_map) {
        if (!page_frame_cache_is_empty(&m->cache)) {
            phys_addr = page_frame_cache_pop(&m->cache);

            bitmap_set(&m->bitmap, (phys_addr - m->base_addr) / PAGE_SIZE);
            break;
        }

        // Find first unallocated page frame
        for (u64 i = 0; i
             < ALIGN_UP(m->bitmap.size, BITMAP_CHUNK_BITS) / BITMAP_CHUNK_BITS;
             ++i) {
            if (m->bitmap.chunks[i] != 0xffffffffffffffff) {
                for (u64 j = 0; j < BITMAP_CHUNK_BITS; j++) {
                    if (((m->bitmap.chunks[i] >> j) & 1) == 0) {
                        m->bitmap.chunks[i] |= (1UL << j);

                        phys_addr = m->base_addr
                            + (i * BITMAP_CHUNK_BITS + j) * PAGE_SIZE;

                        break;
                    }
                }
            }
        }
    }

    return phys_addr;
}

// Free the frame at the given physical address.
// Panic if invalid address is passed.
void pmm_free(u64 phys_addr) {
    kassert(phys_addr % PAGE_SIZE == 0);

    list_for_each(m, memory_map) {
        if (phys_addr >= m->base_addr && phys_addr < m->base_addr + m->len) {
            if (!page_frame_cache_is_full(&m->cache)) {
                page_frame_cache_push(&m->cache,
                                      ALIGN_DOWN(phys_addr, PAGE_SIZE));
            }

            bitmap_clear(&m->bitmap, (phys_addr - m->base_addr) / PAGE_SIZE);
            return;
        }
    }

    kpanic(
        "pmm: Cannot free frame at 0x%016lx which is outside of mapped memory "
        "regions\n",
        phys_addr);
}
