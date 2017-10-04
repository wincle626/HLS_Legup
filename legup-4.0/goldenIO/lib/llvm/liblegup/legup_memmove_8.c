#include "legup/intrinsics.h"

void legup_memmove_8(uint64_t * d, const uint64_t * s, size_t n)
{
    if (d < s) {
        uint64_t * dt = d;
        const uint64_t * st = s;
        n >>= 3;
        while (n--)
            *dt++ = *st++;
    } else if (d > s) {
        n >>= 3;
        uint64_t * dt = d + n;
        const uint64_t * st = s + n;
        while (n--)
            *--dt = *--st;
    }
}
