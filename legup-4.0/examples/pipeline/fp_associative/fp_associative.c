#include <stdio.h>

#define FLT2HEX(a) (*(unsigned*)&a)
#define DBL2HEX(a) (*(unsigned long long*)&a)
#define fabs(a) (((a) >= 0.0f) ? (a) : -(a))

#define N 10
volatile float a[N] = {1,2,3,4,5,6,7,8,9,10};
volatile float b[N] = {1,2,3,4,5,6,7,8,9,10};
volatile float c[N] = {1,2,3,4,5,6,7,8,9,10};
volatile float d[N] = {1,2,3,4,5,6,7,8,9,10};

// Simple accumulation
int main() {
    float sum = 0;
    float sum2 = 0;
    int i;
loop: for (i = 0; i < N; i++) {

    // huge potential range of IIs when pipelining:

    // 14:
    //sum += a[i] + b[i] + c[i] + d[i] + i;
    // 74:
    //sum = sum + a[i] + b[i] + c[i] + d[i] + i;
    // 74:
    //sum = a[i] + sum + b[i] + c[i] + d[i] + i;
    // 59:
    //sum = a[i] + b[i] + sum + c[i] + d[i] + i;
    // 44:
    //sum = a[i] + b[i] + c[i] + sum + d[i] + i;
    // 29:
    sum = a[i] + b[i] + c[i] + d[i] + sum + i;
    // 14:
    //sum = a[i] + b[i] + c[i] + d[i] + i + sum;


    /* two cycles in the dependency graph (pipelineDFG.ps)
    sum += a[i] + b[i] + sum2 + sum;
    sum2 = sum + c[i] + d[i];
    */
    printf("sum %d = %x\n", i, FLT2HEX(sum));
}
    printf("sum = %x\n", FLT2HEX(sum));

    //printf("sum = %f\n", sum);

    float exp = 265.0f;

    printf("Final value is %x\n", FLT2HEX(sum));
    printf("Expected is %x\n", FLT2HEX(exp));

    float sub = sum-exp;
    float diff = fabs(sub);
    printf("sub is %x\n", FLT2HEX(sub));
    printf("Diff is %x\n", FLT2HEX(diff));
    float eps = 1e-6f;
    printf("eps is %x\n", FLT2HEX(eps));

    int equal = diff <= 1e-6f;

    printf("equal is %x\n", equal);

    if (equal) {
        printf("PASSED\n");
    } else {
        printf("Value is %f\n", sum);
        printf("FAILED\n");
    }

    return sum;
}
