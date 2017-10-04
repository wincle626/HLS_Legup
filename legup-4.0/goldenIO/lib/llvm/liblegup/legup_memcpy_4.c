#include "legup/intrinsics.h"

void legup_memcpy_4(uint32_t * d, const uint32_t * s, size_t n)
{
    uint32_t * dt = d;
    const uint32_t * st = s;
    n >>= 2;
    while (n--)
        *dt++ = *st++;
}

