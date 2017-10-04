#include "legup/intrinsics.h"

void legup_memmove_2(uint16_t * d, const uint16_t * s, size_t n)
{
    if (d < s) {
        uint16_t * dt = d;
        const uint16_t * st = s;
        n >>= 1;
        while (n--)
            *dt++ = *st++;
    } else if (d > s) {
        n >>= 1;
        uint16_t * dt = d + n;
        const uint16_t * st = s + n;
        while (n--)
            *--dt = *--st;
    }
}
