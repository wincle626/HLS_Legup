#define N 400

#include <stdio.h>
#include "dividers.h"

int divisor[] = {23412, 45064, 2141, 3262};
volatile int output[N];

int div(int divisor) {
    int i, sum=0;
    unsigned result=0;
    for (i = 0; i < N; i++) {
        result = TEST_INPUTS[i] / divisor;
        output[i] = result;
        sum += result;
    }
//    printf("result = %d\n", result);
    return sum;
}


int main() {
    legup_start_counter(0);
    int i, sum=0;

    for (i=0; i<4; i++) {
        sum += div(divisor[i]);
    }

    int perf_counter = legup_stop_counter(0);
    printf("perf_counter = %d\n", perf_counter);
    // check the result
    printf("Sum: %d\n", sum);
    if (sum == 17690187) {
        printf("RESULT: PASS\n");
    } else {
        printf("RESULT: FAIL\n");
    }
    return sum;
}

