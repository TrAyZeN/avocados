#include "mem.h"

// TODO: Avoid optimization ??
// TODO: volatile
void memset(u8 *mem, u8 value, u64 n) {
    for (u64 i = 0; i < n; ++i) {
        mem[i] = value;
    }
}
