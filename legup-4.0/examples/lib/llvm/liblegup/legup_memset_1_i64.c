#include "legup/intrinsics.h"

void legup_memset_1_i64(uint8_t *m, uint8_t c1, uint64_t n) {
    uint8_t c = c1;

    uint8_t *s = (uint8_t *)m;
    while (n--) {
        *s++ = c;
    }
}
