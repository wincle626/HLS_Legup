#include "legup/intrinsics.h"

void legup_memmove_8_i64(uint8_t *d, const uint8_t *s, uint64_t n) {
    if (d < s) {
        uint64_t *dt = (uint64_t *)d;
        const uint64_t *st = (const uint64_t *)s;
        n >>= 3;
        while (n--)
            *dt++ = *st++;
    } else if (d > s) {
        n >>= 3;
        uint64_t *dt = (uint64_t *)d + n;
        const uint64_t *st = (const uint64_t *)s + n;
        while (n--)
            *--dt = *--st;
    }
}
