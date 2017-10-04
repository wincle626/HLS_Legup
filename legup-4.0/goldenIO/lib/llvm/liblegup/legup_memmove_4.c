#include "legup/intrinsics.h"

void legup_memmove_4(uint32_t * d, const uint32_t * s, size_t n)
{
    if (d < s) {
        uint32_t * dt = d;
        const uint32_t * st = s;
        n >>= 2;
        while (n--)
            *dt++ = *st++;
    } else if (d > s) {
        n >>= 2;
        uint32_t * dt = d + n;
        const uint32_t * st = s + n;
        while (n--)
            *--dt = *--st;
    }
}
