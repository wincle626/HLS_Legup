/* Parallelizable Dot Product Benchmark
 * 
 * A.B=A0*B0+A1*B1+A2*B2+A3*B3
 * 
 * 
 */

#define ARRAY_SIZE 6000
#define NUM_ACCEL 6
#define OPS_PER_ACCEL ARRAY_SIZE/NUM_ACCEL

#include <stdio.h>
#include <pthread.h>
#include "dotproduct.h"

//int result_array[NUM_ACCEL];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int final_result=0;

struct thread_data{
   int  startidx;
   int  maxidx;
};

// Calculate the values for an entire row
void *product(void *threadarg)
{
	int i;
	int result = 0;
	struct thread_data* arg = (struct thread_data*) threadarg;
	int startidx = arg->startidx;
	int maxidx = arg->maxidx;
	for (i = startidx; i < maxidx; i++){
		result+= A_array[i]*B_array[i];
//		printf("index i = %d\n", i);
	}
	
	//printf("startidx = %d, endidx = %d, result = %d\n", startidx, maxidx, result);
	pthread_mutex_lock (&mutex);
	final_result += result;
	pthread_mutex_unlock (&mutex);
	pthread_exit(NULL);
}

int main(){
	int i;
	//create the thread variables
	pthread_t threads[NUM_ACCEL];
	struct thread_data data[NUM_ACCEL];

	for (i=0; i<NUM_ACCEL; i++) {
		data[i].startidx = i*OPS_PER_ACCEL;
		data[i].maxidx = (i+1)*OPS_PER_ACCEL;
	}

	//fork the threads
	for (i=0; i<NUM_ACCEL; i++) {
		//data[i].startidx = i*OPS_PER_ACCEL;
		//data[i].maxidx = (i+1)*OPS_PER_ACCEL;
		//product (A_array, B_array, i*OPS_PER_ACCEL, (i+1)*OPS_PER_ACCEL);
		pthread_create(&threads[i], NULL, product, (void *)&data[i]);
	}

	//join the threads
	for (i=0; i<NUM_ACCEL; i++) {
		pthread_join(threads[i], NULL);
	}

	//check final result
	printf ("Result: %d\n", final_result);
	if (final_result == 60799800) {
		printf("RESULT: PASS\n");
	} else {
		printf("RESULT: FAIL\n");
	}
	return 0;
}
