#define N 10000

#include <stdio.h>
#include "dividers.h"

//#define LEGUP 1

int div() {
    int i, sum=0;
    int result=0;
    for (i = 0; i < N; i++) {
        result = TEST_INPUTS[i] / TEST_INPUTS2[i];
        sum += result;
    }
    return sum;
}


int main() {
    #ifdef LEGUP
    legup_start_counter(0);
    #endif
    int i, sum=0;

    sum = div();

    #ifdef LEGUP
    int perf_counter = legup_stop_counter(0);
    printf("perf_counter = %d\n", perf_counter);
    #endif
    // check the result
    printf("Sum: %d\n", sum);
    if (sum == 578900) {
        printf("RESULT: PASS\n");
    } else {
        printf("RESULT: FAIL\n");
    }
    return sum;
}

