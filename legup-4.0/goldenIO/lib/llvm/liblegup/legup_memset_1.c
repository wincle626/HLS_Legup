#include "legup/intrinsics.h"

void legup_memset_1(uint8_t *m, uint32_t c1, size_t n)
{
    // only use the first byte
    uint8_t c = c1 & 0xff;

    uint8_t *s = (uint8_t *) m;
    while (n--) {
        *s++ = c;
    }
}
