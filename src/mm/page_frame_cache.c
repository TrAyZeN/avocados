#include "libk/kassert.h"
#include "page_frame_cache.h"
#include "pmm.h"
#include "tools/test.h"

bool page_frame_cache_is_empty(page_frame_cache_t *cache) {
    return cache->count == 0;
}

bool page_frame_cache_is_full(page_frame_cache_t *cache) {
    return cache->count == PAGE_FRAME_CACHE_SIZE;
}

void page_frame_cache_push(page_frame_cache_t *cache, u64 frame_addr) {
    kassert(!page_frame_cache_is_full(cache));
    kassert(frame_addr % PAGE_SIZE == 0);

    cache->frame_addrs[cache->count] = frame_addr;
    cache->count += 1;
}

u64 page_frame_cache_pop(page_frame_cache_t *cache) {
    kassert(!page_frame_cache_is_empty(cache));

    cache->count -= 1;
    return cache->frame_addrs[cache->count];
}

DEFINE_TEST(test_page_frame_cache) {
    page_frame_cache_t cache = { .count = 0 };

    kassert(page_frame_cache_is_empty(&cache));

    for (u64 i = 0; i < PAGE_FRAME_CACHE_SIZE; ++i) {
        kassert(!page_frame_cache_is_full(&cache));
        page_frame_cache_push(&cache, i * PAGE_SIZE);
        kassert(!page_frame_cache_is_empty(&cache));
    }

    kassert(page_frame_cache_is_full(&cache));

    for (u64 i = 0; i < PAGE_FRAME_CACHE_SIZE; ++i) {
        kassert(!page_frame_cache_is_empty(&cache));
        kassert(page_frame_cache_pop(&cache)
                == (PAGE_FRAME_CACHE_SIZE - 1 - i) * PAGE_SIZE);
        kassert(!page_frame_cache_is_full(&cache));
    }
}
