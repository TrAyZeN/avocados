#include "utils.h"

// TODO: Avoid optimization ??
// TODO: volatile
void memset(uint8_t *mem, uint8_t value, uint64_t n) {
    for (uint64_t i = 0; i < n; ++i) {
        mem[i] = value;
    }
}

int strncmp(const char *s1, const char *s2, uint64_t n) {
    uint64_t i = 0;
    while (i < n && s1[i] == s2[i] && s1[i] != '\0') {
        i += 1;
    }

    return i == n ? 0 : s1[i] - s2[i];
}
