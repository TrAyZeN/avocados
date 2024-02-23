#include "attributes.h"
#include "bitmap.h"
#include "kassert.h"
#include "tools/test.h"

void bitmap_set(bitmap_t *bitmap, u64 idx) {
    kassert(idx < bitmap->size);

    bitmap->chunks[idx / BITMAP_CHUNK_BITS] |= 1UL << (idx % BITMAP_CHUNK_BITS);
}

void bitmap_clear(bitmap_t *bitmap, u64 idx) {
    kassert(idx < bitmap->size);

    bitmap->chunks[idx / BITMAP_CHUNK_BITS] &=
        ~(1UL << (idx % BITMAP_CHUNK_BITS));
}

void bitmap_set_range(bitmap_t *bitmap, u64 start, u64 count) {
    kassert(start < bitmap->size && start + count <= bitmap->size);
    // Overflow of start + count is okay since it won't loop

    for (u64 i = start; i < start + count; ++i) {
        bitmap_set(bitmap, i);
    }
}

void bitmap_clear_range(bitmap_t *bitmap, u64 start, u64 count) {
    kassert(start < bitmap->size && start + count <= bitmap->size);
    // Overflow of start + count is okay since it won't loop

    for (u64 i = start; i < start + count; ++i) {
        bitmap_clear(bitmap, i);
    }
}

DEFINE_TEST(test_bitmap) {
    __align(_Alignof(bitmap_t))
        u8 mem[sizeof(bitmap_t) + 2 * sizeof(u64)] = { 0 };
    bitmap_t *bitmap = (bitmap_t *)mem;
    bitmap->size = 125;

    bitmap_set_range(bitmap, 62, 4);
    kassert(bitmap->chunks[0] == ((1UL << 62) | (1UL << 63)));
    kassert(bitmap->chunks[1] == ((1UL << 0) | (1UL << 1)));

    bitmap_clear_range(bitmap, 63, 2);
    kassert(bitmap->chunks[0] == (1UL << 62));
    kassert(bitmap->chunks[1] == (1UL << 1));
}
