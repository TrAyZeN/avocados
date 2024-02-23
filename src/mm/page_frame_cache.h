#ifndef AVOCADOS_PAGE_FRAME_CACHE_H_
#define AVOCADOS_PAGE_FRAME_CACHE_H_

#include <stdbool.h>

#include "types.h"

#define PAGE_FRAME_CACHE_SIZE 16

// Stack of page frames for faster free/alloc
typedef struct {
    // Number of frames in the stack
    u64 count;
    // Frame addresses
    u64 frame_addrs[PAGE_FRAME_CACHE_SIZE];
} page_frame_cache_t;

bool page_frame_cache_is_empty(page_frame_cache_t *cache);
bool page_frame_cache_is_full(page_frame_cache_t *cache);
void page_frame_cache_push(page_frame_cache_t *cache, u64 frame_addr);
u64 page_frame_cache_pop(page_frame_cache_t *cache);

#endif /* ! AVOCADOS_PAGE_FRAME_CACHE_H_ */
