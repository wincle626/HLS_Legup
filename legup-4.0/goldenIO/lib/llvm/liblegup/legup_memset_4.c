#include "legup/intrinsics.h"

void legup_memset_4(uint32_t *m, uint32_t c, size_t n)
{
    // only use the first byte
    c &= 0xff;

    c |= (c << 8);
    c |= (c << 16);

    uint32_t *s = (uint32_t *) m;
    n >>= 2;
    while (n--)
        *s++ = c;
}
