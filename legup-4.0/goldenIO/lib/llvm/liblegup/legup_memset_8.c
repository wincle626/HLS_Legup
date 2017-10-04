#include "legup/intrinsics.h"

void legup_memset_8(uint64_t *m, uint32_t c1, size_t n)
{
    // only use the first byte
    uint64_t c = c1 & 0xff;

    c |= (c << 8);
    c |= (c << 16);
    c |= (c << 32);

    uint64_t *s = (uint64_t *) m;
    n >>= 3;
    while (n--)
        *s++ = c;
}

