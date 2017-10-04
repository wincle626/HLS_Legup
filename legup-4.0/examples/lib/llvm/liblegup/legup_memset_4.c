#include "legup/intrinsics.h"

void legup_memset_4(uint8_t *m, uint8_t c1, uint32_t n) {
    uint32_t c = c1;

    c |= (c << 8);
    c |= (c << 16);

    uint32_t *s = (uint32_t *) m;
    n >>= 2;
    while (n--)
        *s++ = c;
}
