#ifndef AVOCADOS_BITMAP_H_
#define AVOCADOS_BITMAP_H_

#include "types.h"

typedef struct {
    // Number of bits the bitmap can contain
    u64 size;
    // The chunks array length should be at least
    // (size + BITMAP_CHUNK_BITS - 1) / BITMAP_CHUNK_BITS
    u64 chunks[];
} bitmap_t;

#define BITS_PER_BYTE 8
#define BITMAP_CHUNK_BITS (sizeof(((bitmap_t *)0)->chunks[0]) * BITS_PER_BYTE)

void bitmap_set(bitmap_t *bitmap, u64 idx);
void bitmap_clear(bitmap_t *bitmap, u64 idx);

void bitmap_set_range(bitmap_t *bitmap, u64 start, u64 count);
void bitmap_clear_range(bitmap_t *bitmap, u64 start, u64 count);

#endif /* ! AVOCADOS_BITMAP_H_ */
