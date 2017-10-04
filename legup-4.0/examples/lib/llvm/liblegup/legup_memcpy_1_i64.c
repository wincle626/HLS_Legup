#include "legup/intrinsics.h"

void legup_memcpy_1_i64(uint8_t *d, const uint8_t *s, uint64_t n) {
    uint8_t *dt = d;
    const uint8_t *st = s;
    while (n--)
        *dt++ = *st++;
}
