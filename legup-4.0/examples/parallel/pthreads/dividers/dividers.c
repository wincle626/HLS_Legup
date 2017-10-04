#define N 10000
#define NUM_ACCEL 20
#define OPS_PER_ACCEL N/NUM_ACCEL

#include <stdio.h>
#include "dividers.h"
#include <pthread.h>

//#define LEGUP 1

struct thread_data{
   int  startidx;
   int  maxidx;
};

void *div(void* threadarg) {
    int i, j, tid, divisor;
    int total=0;
    int result;
	struct thread_data* arg = (struct thread_data*) threadarg;
	int startidx = arg->startidx;
	int maxidx = arg->maxidx;
    
    for (i = startidx; i < maxidx; i++) {
        result = TEST_INPUTS[i] / TEST_INPUTS2[i];
//        printf("result[%d] = %d\n", i, result);
        total += result;
    }
        
    //printf("total = %d\n", total);
                  
    pthread_exit((void*)total);
}


int main() {

    #ifdef LEGUP
    legup_start_counter(0);
    #endif
    int i, sum=0;
	int result[NUM_ACCEL];
	pthread_t threads[NUM_ACCEL];
	struct thread_data data[NUM_ACCEL];

	//initialize structs to pass into accels
	for (i=0; i<NUM_ACCEL; i++) {
		data[i].startidx = i*OPS_PER_ACCEL;
        data[i].maxidx = (i+1)*OPS_PER_ACCEL;
	}

    //launch threads
	for (i=0; i<NUM_ACCEL; i++) {
		pthread_create(&threads[i], NULL, div, (void *)&data[i]);
	}
	 
	//join the threads
	for (i=0; i<NUM_ACCEL; i++) {
		pthread_join(threads[i], (void**)&result[i]);
	}

	for (i=0; i<NUM_ACCEL; i++) {
        sum += result[i];
        //printf("result[%d] = %d\n", i, result[i]);
	}

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

