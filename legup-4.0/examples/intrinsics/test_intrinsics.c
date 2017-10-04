#include <stdio.h>
#include "legup/intrinsics.h"

// Test memset and memcpy align 4, defined in intrinsics.h
// Expected result: 
//      array: 16843009 16843009 16843009 
//      sum: 50529027
//
// We fill the array with a byte containing: 00000001
// Note that:
//      1 + (1 << 8) + (1 << 16) + (1 << 24)
//      = 16843009

#define LENGTH 3
int main() {
    unsigned int array[LENGTH] = {1, 2, 3}, array2[LENGTH];
    unsigned int i, sum = 0;

    // word-aligned memset and memcpy
    memset_4((unsigned char *)array, 1, sizeof(array));
    memcpy_4((unsigned char *)array2, (unsigned char *)array, sizeof(array));

    printf("array: ");
    for (i = 0; i < LENGTH; i++) {
        printf("%d ", array2[i]);
        sum += array2[i];
    }
    printf("\n");
    printf("Result: %d\n", sum);
    if (sum == 50529027) {
        printf("RESULT: PASS\n");
    } else {
        printf("RESULT: FAIL\n");
    }
    return sum;
}
