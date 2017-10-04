#include "legup/intrinsics.h"

void legup_memmove_1(uint8_t * d, const uint8_t * s, size_t n)
{
    if (d < s) {
        uint8_t * dt = d;
        const uint8_t * st = s;
        while (n--)
            *dt++ = *st++;
    } else if (d > s) {
        uint8_t * dt = d + n;
        const uint8_t * st = s + n;
        while (n--)
            *--dt = *--st;
    }
}
