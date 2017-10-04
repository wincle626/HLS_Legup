#include "legup/intrinsics.h"

void legup_memset_2_i64(uint8_t *m, uint8_t c1, uint64_t n) {
    uint16_t c = c1;

    c |= (c << 8);

    uint16_t *s = (uint16_t *)m;
    n >>= 1;
    while (n--)
        *s++ = c;
}
