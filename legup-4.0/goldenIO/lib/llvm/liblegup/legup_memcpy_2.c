#include "legup/intrinsics.h"

void legup_memcpy_2(uint16_t * d, const uint16_t * s, size_t n)
{
    uint16_t * dt = d;
    const uint16_t * st = s;
    n >>= 1;
    while (n--)
        *dt++ = *st++;
}
