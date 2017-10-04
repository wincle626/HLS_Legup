#define N 400
#define NUM_ACCEL 4

#include <stdio.h>
#include "dividers.h"
#include <pthread.h>

int divisor[] = {23412, 45064, 2141, 3262};
volatile int output[N];

void *div(void* threadarg) {
    int i, sum=0;
    unsigned result=0;
    int divisor = *(int*)threadarg;
    for (i = 0; i < N; i++) {
        result = TEST_INPUTS[i] / divisor;
        output[i] = result;
        sum += result;
    }
  
    pthread_exit((void*)sum); 
}


int main() {

    legup_start_counter(0);
    int i, sum=0;
	int result[NUM_ACCEL];
	pthread_t threads[NUM_ACCEL];

    //launch threads
	for (i=0; i<NUM_ACCEL; i++) {
		pthread_create(&threads[i], NULL, div, (void *)&divisor[i]);
	}
	 
	//join the threads
	for (i=0; i<NUM_ACCEL; i++) {
		pthread_join(threads[i], (void**)&result[i]);
	}

	for (i=0; i<NUM_ACCEL; i++) {
        sum += result[i];
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

