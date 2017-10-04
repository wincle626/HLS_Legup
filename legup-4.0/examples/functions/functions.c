#include <stdio.h>

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

int main() {
    int global = 0;

    Init(&global);

    global += ByVal(10, 10);

    ByRef(&global, 10);

    printf ("Result: %d\n", global);
    if (global == 40) {
        printf("RESULT: PASS\n");
    } else {
        printf("RESULT: FAIL\n");
    }
    return global;
}
