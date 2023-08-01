#include <stddef.h>
#include <stdint.h>

#include "kassert.h"
#include "kprintf.h"
#include "log.h"
#include "paging.h"
#include "pmm.h"
#include "utils.h"

extern uint64_t _skern;
extern uint64_t _ekern;

extern pml4_t pml4;

// TODO: Add free stack for fast alloc and free

struct memory_map {
    struct memory_map *next;
    // Base physical address of the memory region
    uint64_t base_addr;
    // Length in bytes of the memory region
    uint64_t len;
    // Bitmap size in 64 bits chunks
    uint64_t size;
    // A bitmap representing page frame availability. Each bit corresponds to a
    // page frame with 0 = free and 1 = allocated.
    uint64_t bitmap[];
};

#define BITMAP_CELL_BITS (sizeof(((struct memory_map *)0)->bitmap[0]) * 8)

static void memory_map_reserve(struct memory_map *memory_map, uint64_t base,
                               uint64_t num_frames);
static void memory_map_reserve_range(struct memory_map *memory_map,
                                     uint64_t base, uint64_t end);

static struct memory_map *memory_map = NULL;

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
        "PMM: Physical memory outside %lx-%lx will be considered free\n",
        (uint64_t)&_skern, (uint64_t)&_ekern);

    // Copy available memory map entries on the stack (in kernel range)
    uint32_t num_available_mmap_entries = 0;
    struct multiboot_mmap_entry available_mmap_entries[MAX_MMAP_ENTRIES] = {
        0
    };
    for (uint64_t i = 0; sizeof(struct multiboot_tag_mmap)
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

    uint8_t first = 1;
    // WARN: Must be a multiple of PAGE_SIZE
    uint64_t memory_map_virt_addr = MEMORY_MAP_ADDR;
    struct memory_map *prev_memory_map = NULL;
    struct memory_map *curr_memory_map = NULL;
    for (uint64_t i = 0; i < num_available_mmap_entries; i++) {
        // TODO: Function
        // Compute struct memory_map size with flexible array member
        uint64_t memory_map_size = sizeof(struct memory_map)
            + ALIGN_UP(available_mmap_entries[i].len,
                       PAGE_SIZE * BITMAP_CELL_BITS)
                / (PAGE_SIZE * 8);

        // TODO: Improve first check
        // WARN: Maybe _kern is not the first memory region
        uint64_t offset = 0;
        if (curr_memory_map == NULL) {
            // Store the memory map after the kernel range
            offset = ALIGN_UP((uint64_t)&_ekern, PAGE_SIZE);
        }
        uint64_t memory_map_phys_addr =
            ALIGN_UP(available_mmap_entries[i].addr + offset, PAGE_SIZE);

        // Setup paging structures for memory map
        struct pml4e *pml4e = get_pml4e(memory_map_virt_addr);
        if (!pml4e->present) {
            // TODO: Map
            kpanic("Not implemented: pml4e not present\n");
        }

        struct pdpte *pdpte = get_pdpte(memory_map_virt_addr);
        if (!pdpte->present) {
            // TODO: Map
            kpanic("Not implemented: pdpte not present\n");
        }

        uint64_t res = 0;
        struct pde *pde = get_pde(memory_map_virt_addr);
        if (!pde->present) {
            // Store the page table after the memory map
            uint64_t phys_addr =
                ALIGN_UP(memory_map_phys_addr + memory_map_size, PAGE_SIZE)
                + res * PAGE_SIZE;
            *pde = (struct pde){
                .present = 1,
                .rw = 1,
                .us = 0,
                .addr = BIT_RANGE(phys_addr, 12, 51),
                .xd = 1,
            };

            // Zero initialize the table so that present bits are 0
            memset((uint8_t *)phys_addr, 0, PAGE_SIZE);
            res += 1;
        }

        // Assert that we only need one pde
        kassert(memory_map_virt_addr / (PAGE_SIZE * 512)
                == (memory_map_virt_addr + memory_map_size)
                    / (PAGE_SIZE * 512));

        for (uint64_t k = 0; k < ALIGN_UP(memory_map_size, PAGE_SIZE);
             k += PAGE_SIZE) {
            struct pte *pte = get_pte(memory_map_virt_addr + k);

            kassert(!pte->present);
            uint64_t phys_addr = memory_map_phys_addr + k;
            *pte = (struct pte){
                .present = 1,
                .rw = 1,
                .us = 0,
                .addr = BIT_RANGE(phys_addr, 12, 51),
                .xd = 1,
            };
        }

        curr_memory_map = (struct memory_map *)memory_map_virt_addr;
        curr_memory_map->next = NULL;
        curr_memory_map->base_addr = available_mmap_entries[i].addr;
        curr_memory_map->len =
            ALIGN_DOWN(available_mmap_entries[i].len, PAGE_SIZE);
        curr_memory_map->size =
            ALIGN_UP(curr_memory_map->len / PAGE_SIZE, BITMAP_CELL_BITS)
            / BITMAP_CELL_BITS;
        memset((uint8_t *)&curr_memory_map->bitmap, 0,
               curr_memory_map->size * sizeof(curr_memory_map->bitmap[0]));

        kassert(curr_memory_map->len % PAGE_SIZE == 0);
        kassert(curr_memory_map->base_addr % PAGE_SIZE == 0);
        // Assert that curr_memory_map->base_addr + curr_memory_map->len must
        // not overflow
        kassert(curr_memory_map->len
                <= UINT64_MAX - curr_memory_map->base_addr);

        if (memory_map == NULL) {
            memory_map = curr_memory_map;

            memory_map_reserve_range(curr_memory_map,
                                     ALIGN_DOWN((uint64_t)&_skern, PAGE_SIZE),
                                     ALIGN_UP((uint64_t)&_ekern, PAGE_SIZE));
        } else {
            prev_memory_map->next = curr_memory_map;
        }

        // Reserve memory map pages + page table pages (see res)
        memory_map_reserve_range(
            curr_memory_map, memory_map_phys_addr,
            ALIGN_UP(memory_map_phys_addr + memory_map_size, PAGE_SIZE)
                + res * PAGE_SIZE);

        // Bitmap are uint64_t but the memory region length may not be a
        // multiple of PAGE_SIZE * 64, so we need to mark these frames as
        // allocated.
        for (uint64_t frame_idx = curr_memory_map->len / PAGE_SIZE;
             frame_idx < curr_memory_map->size * BITMAP_CELL_BITS;
             ++frame_idx) {
            curr_memory_map->bitmap[frame_idx / BITMAP_CELL_BITS] |= 1UL
                << (frame_idx % BITMAP_CELL_BITS);
        }

        // TODO: Remove that
        if (first) {
            kprintf("%lx\n", curr_memory_map->bitmap[0]);
        }

        prev_memory_map = curr_memory_map;
        memory_map_virt_addr += ALIGN_UP(memory_map_size, PAGE_SIZE);
        first = 0;
    }

    log(LOG_LEVEL_INFO, "PMM: PMM initialized\n");
}

// base is a physical address
static void memory_map_reserve(struct memory_map *m, uint64_t base,
                               uint64_t num_frames) {
    // Check alignment
    kassert(base % PAGE_SIZE == 0);
    // Check range
    kassert(base >= m->base_addr
            && base + num_frames * PAGE_SIZE < m->base_addr + m->len);

    uint64_t base_frame = (base - m->base_addr) / PAGE_SIZE;
    for (uint64_t frame_idx = 0; frame_idx < num_frames; ++frame_idx) {
        m->bitmap[(base_frame + frame_idx) / BITMAP_CELL_BITS] |= 1UL
            << ((base_frame + frame_idx) % BITMAP_CELL_BITS);
    }
}

// end excluded
static void memory_map_reserve_range(struct memory_map *m, uint64_t base,
                                     uint64_t end) {
    // Check alignment
    kassert(end % PAGE_SIZE == 0);
    // Check range
    kassert(base < end);

    return memory_map_reserve(m, base, (end - base) / PAGE_SIZE);
}

// Return the physical address of the allocated frame
// If unable to find a free frame, returns PMM_ALLOC_ERROR.
uint64_t pmm_alloc(void) {
    uint64_t phys_addr = PMM_ALLOC_ERROR;

    for (struct memory_map *m = memory_map; m != NULL; m = m->next) {
        for (uint64_t i = 0; i < m->size; i++) {
            if (m->bitmap[i] != 0xffffffffffffffff) {
                // Find first bit
                for (uint64_t j = 0; j < BITMAP_CELL_BITS; j++) {
                    if (((m->bitmap[i] >> j) & 1) == 0) {
                        m->bitmap[i] |= (1UL << j);

                        phys_addr = m->base_addr
                            + (i * BITMAP_CELL_BITS + j) * PAGE_SIZE;

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
void pmm_free(uint64_t addr) {
    kassert(addr % PAGE_SIZE == 0);

    for (struct memory_map *m = memory_map; m != NULL; m = m->next) {
        if (addr >= m->base_addr && addr < m->base_addr + m->len) {
            uint64_t frame_idx = (addr - m->base_addr) / PAGE_SIZE;
            m->bitmap[frame_idx / BITMAP_CELL_BITS] &=
                ~(1UL << (frame_idx % BITMAP_CELL_BITS));
            return;
        }
    }

    kpanic("pmm: Cannot free frame at %lx which is outside of mapped memory "
           "regions\n",
           addr);
}
