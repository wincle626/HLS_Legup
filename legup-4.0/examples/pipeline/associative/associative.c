#include <stdio.h>

#define N 10
volatile int a[N] = {1,2,3,4,5,6,7,8,9,10};
volatile int b[N] = {1,2,3,4,5,6,7,8,9,10};
volatile int c[N] = {1,2,3,4,5,6,7,8,9,10};
volatile int d[N] = {1,2,3,4,5,6,7,8,9,10};

// Simple accumulation
int main() {
    int sum = 0;
    int sum2 = 0;
    int i;
loop: for (i = 0; i < N; i++) {
    sum += a[i] + b[i] + c[i] + d[i] + i;
    /* two cycles in the dependency graph (pipelineDFG.ps)
    sum += a[i] + b[i] + sum2 + sum;
    sum2 = sum + c[i] + d[i];
    */
    printf("sum %d = %d\n", i, sum);
}
    printf("sum = %d\n", sum);

    if (sum == 265) {
        printf("PASSED\n");
    } else {
        printf("FAILED\n");
    }

    return sum;
}
