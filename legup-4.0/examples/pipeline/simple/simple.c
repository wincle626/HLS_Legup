#include <stdio.h>

// Simple loop with an array
int main() {
    int i;

#define N 4
    volatile int a[N] = {1,2,3,4};
    volatile int b[N] = {5,6,7,8};
    volatile int c[N] = {0};

    loop: for (i = 0; i < N; i++) {
        printf("Loop body\n");
        printf("a[%d] = %d\n", i, a[i]);
        printf("b[%d] = %d\n", i, b[i]);
        c[i] = a[i] + b[i];
        printf("c[%d] = %d\n", i, c[i]);
    }

    int sum = 0;
    for (i = 0; i < N; i++) {
        printf("c[%d] = %d\n", i, c[i]);
        sum += c[i];
    }

    return sum;
}
