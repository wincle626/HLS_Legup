#include <stdio.h>

int main() {
//#define N 100
//#define SQRT 10
#define N 9
#define SQRT 3

    int A[N], B[N];
    long long C[N];
    int i, j;
    long long tmp;

    for(i = 0; i < N; i++) {
        A[i] = i;
    }

    int sum1 = 0;
    Loop1: for(i = 0; i < N; i++) {
        tmp = 2 + A[i];
        tmp = tmp * tmp + tmp - 1;
        B[i] = tmp * 3 + 1;
        sum1 += B[i];
        printf("sum %d = %d\n", i, sum1);
    }

    long long sum = 0;
    for(i = 0; i < SQRT; i++) {
        Loop2: for(j = 0; j < SQRT; j++) {
            int idx = i * SQRT + j;
            tmp = 2 + B[idx];
            tmp = tmp * tmp * tmp + tmp * tmp + 10;
            C[idx] = (tmp + 10) * 2;
            printf("C[%d] = %lld\n", idx, C[idx]);
            sum += C[idx];
            printf("sum %d = %lld\n", idx, sum);
        }
    }

    printf("sum = %lld\n", sum);

    return sum;
}
