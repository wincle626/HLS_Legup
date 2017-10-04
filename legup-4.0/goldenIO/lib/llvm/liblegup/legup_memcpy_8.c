#include "legup/intrinsics.h"

void legup_memcpy_8(uint64_t * d, const uint64_t * s, size_t n)
{
    uint64_t * dt = d;
    const uint64_t * st = s;
    n >>= 3;
    while (n--)
        *dt++ = *st++;
}

