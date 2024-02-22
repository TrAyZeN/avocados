#include "string.h"

int strncmp(const char *s1, const char *s2, u64 n) {
    u64 i = 0;
    while (i < n && s1[i] == s2[i] && s1[i] != '\0') {
        i += 1;
    }

    return i == n ? 0 : s1[i] - s2[i];
}

u64 strlen(const char *s) {
    u64 len = 0;

    while (*s != '\0') {
        len += 1;
        s += 1;
    }

    return len;
}
