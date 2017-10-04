// Test unaligned memory access.
// Should fail with an error

#include <stdio.h>
int main() {
    // two integers (8 bytes)
    volatile int a[2] = {0xAABBCCDD, 0xEEFF0011};
    printf("Two integers: %x %x\n", a[0], a[1]);

    // grab pointer to the first byte
    char *b = (char*)&a[0];

    int i;
    for (i = 0; i < 8; i++) {
        int *c = (int*)b;
        printf("Byte %d: %x\n", i, *c);
        b++;
    }
    return 0;
}
