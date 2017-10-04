#include <stdio.h>

#define N 10
volatile int a[N] = {1,2,3,4,5,6,7,8,9,10};
volatile int b[N] = {1,2,3,4,5,6,7,8,9,10};
volatile int c[N] = {1,2,3,4,5,6,7,8,9,10};
volatile int d[N] = {1,2,3,4,5,6,7,8,9,10};
volatile int e[N] = {1,2,3,4,5,6,7,8,9,10};
volatile int f[N] = {1,2,3,4,5,6,7,8,9,10};
volatile int g[N] = {1,2,3,4,5,6,7,8,9,10};
volatile int h[N] = {1,2,3,4,5,6,7,8,9,10};
volatile int ii[N] = {1,2,3,4,5,6,7,8,9,10};
volatile int j[N] = {1,2,3,4,5,6,7,8,9,10};

// Simple accumulation
int main() {
    int sum = 0;
    int sum2 = 0;
    int i;
loop: for (i = 0; i < N; i++) {
    sum += a[i] + b[i] + c[i] + d[i] + 
           e[i] + f[i] - g[i] + h[i] + 
           ii[i] + j[i] - i;

    printf("sum %d = %d\n", i, sum);
}
    sum *= a[4];
    printf("sum = %d\n", sum);

    if (sum == 1975) {
        printf("PASSED\n");
    } else {
        printf("FAILED\n");
    }

    return sum;
}
