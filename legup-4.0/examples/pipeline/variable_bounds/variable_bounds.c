#include <stdio.h>

#define N 4
volatile int a[N] = {1,2,3,4};
volatile int b[N] = {5,6,7,8};
volatile int c[N] = {0};

int variable_bound(int bound) {
    printf("calling variable_bound(%d)\n", bound);
    int i;
    int sum = 0;
    loop: for (i = 0; i < bound; i++) {
        printf("Loop body\n");
        printf("a[%d] = %d\n", i, a[i]);
        printf("b[%d] = %d\n", i, b[i]);
        sum += a[i] + b[i];
    }
    printf("sum = %d\n", sum);
    return sum;
}

// Simple loop with an array
int main() {
    int i;

    int sum = 0;
    sum += variable_bound(0);
    sum += variable_bound(1);
    sum += variable_bound(2);
    sum += variable_bound(3);
    sum += variable_bound(4);

    printf("final sum = %d\n", sum);
    return sum;
}

