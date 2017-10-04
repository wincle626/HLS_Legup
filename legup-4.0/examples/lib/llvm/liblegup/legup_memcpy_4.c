#include "legup/intrinsics.h"

void legup_memcpy_4(uint8_t *d, const uint8_t *s, uint32_t n) {
    uint32_t *dt = (uint32_t *)d;
    const uint32_t *st = (const uint32_t *)s;
    n >>= 2;
    while (n--)
        *dt++ = *st++;
}

