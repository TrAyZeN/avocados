#include "utils.h"

// TODO: Avoid optimization ??
// TODO: volatile
void memset(uint8_t *mem, uint8_t value, uint64_t n) {
    for (uint64_t i = 0; i < n; ++i) {
        mem[i] = value;
    }
}
