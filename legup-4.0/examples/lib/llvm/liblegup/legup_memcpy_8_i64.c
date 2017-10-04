#include "legup/intrinsics.h"

void legup_memcpy_8_i64(uint8_t *d, const uint8_t *s, uint64_t n) {
    uint64_t *dt = (uint64_t *)d;
    const uint64_t *st = (const uint64_t *)s;
    n >>= 3;
    while (n--)
        *dt++ = *st++;
}
