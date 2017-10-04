#include <stdio.h>

// Simple loop with an array
int main() {
    int i;

#define N 10
    volatile int a[N] = {1,2,3,4,5,6,7,8,9,10};
    volatile int b[N] = {11,12,13,14,15,16,17,18,19,20};
    volatile int c[N] = {22,22,23,24,25,26,27,28,29,20};

    int sum = 0;
    loop: for (i = 1; i < N-1; i++) {
        printf("Loop body\n");
        printf("a[%d] = %d\n", i, a[i]);
        printf("b[%d] = %d\n", i, b[i]);
        c[i] *= a[i+1] + b[i-1];
        printf("c[%d] = %d\n", i, c[i]);
        sum += c[i];
        printf("sum = %d\n", sum);
    }

    for (i = 0; i < N; i++) {
        printf("c[%d] = %d\n", i, c[i]);
    }

    printf("sum = %d\n", sum);
    return sum;
}
