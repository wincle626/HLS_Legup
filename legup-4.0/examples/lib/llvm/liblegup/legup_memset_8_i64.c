#include "legup/intrinsics.h"

void legup_memset_8_i64(uint8_t *m, uint8_t c1, uint64_t n) {
    uint64_t c = c1;

    c |= (c << 8);
    c |= (c << 16);
    c |= (c << 32);

    uint64_t *s = (uint64_t *)m;
    n >>= 3;
    while (n--)
        *s++ = c;
}
