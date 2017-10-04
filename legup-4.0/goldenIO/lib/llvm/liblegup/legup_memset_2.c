#include "legup/intrinsics.h"

void legup_memset_2(uint16_t *m, uint32_t c1, size_t n)
{
    // only use the first byte
    uint16_t c = c1 & 0xff;

    c |= (c << 8);

    uint16_t *s = (uint16_t *) m;
    n >>= 1;
    while (n--)
        *s++ = c;
}
