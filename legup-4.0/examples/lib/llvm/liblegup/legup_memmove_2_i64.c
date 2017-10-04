#include "legup/intrinsics.h"

void legup_memmove_2_i64(uint8_t *d, const uint8_t *s, uint64_t n) {
    if (d < s) {
        uint16_t *dt = (uint16_t *)d;
        const uint16_t *st = (const uint16_t *)s;
        n >>= 1;
        while (n--)
            *dt++ = *st++;
    } else if (d > s) {
        n >>= 1;
        uint16_t *dt = (uint16_t *)d + n;
        const uint16_t *st = (const uint16_t *)s + n;
        while (n--)
            *--dt = *--st;
    }
}
