#include <stdio.h>

#include "legup_mem.h"

// Testing multiple functions
// Expected result: 40

void Init(int *a) {
    *a = 10;
}

int ByVal(int a, int b) {
    return a + b;
}

// Note: a is both an input and an output
void ByRef(int *a, int b) {
    *a = *a + b;
}

void ByRef_sw(int *a, int b) {
    *a = *a + b;
}

int main() {
    int global = 0;
    int compare = 0;

    Init(&global);
    Init(&compare);

    int i, max = 10, increment = 123;
    printf("Incrementing by %d for %d iterations:\n", increment, max);

    int *SHARED_MEM_global = malloc_shared(sizeof(int), &global, LEGUP_RAM_LOCATION_ONCHIP);

    for (i = 0; i<max; i++) {
        // pre-accelerator copy
        memcpy_to_shared(SHARED_MEM_global, &global, sizeof(int));
        // accelerator call
        ByRef(SHARED_MEM_global, increment);
	// post-accelerator copy
        memcpy_from_shared(&global, SHARED_MEM_global, sizeof(int));

        ByRef_sw(&compare, increment);

        printf ("Result: %d\n", global);
        if (global == compare) {
            printf("RESULT: PASS\n");
        } else {
            printf("RESULT: FAIL\n");
        }
    }

    free_shared(SHARED_MEM_global);

    return global;
}
