#define NUM_ACCEL 6
#define MAX_NUM 100
#define NUM_BINS 5
#define BIN_MAX_NUM MAX_NUM/NUM_BINS
#define ARRAY_SIZE 36000
#define OPS_PER_ACCEL ARRAY_SIZE/NUM_ACCEL

#include <stdio.h>
#include <pthread.h>
#include "histogram.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int final_result[NUM_BINS] = {0, 0, 0, 0, 0};

struct thread_data{
   int  startidx;
   int  maxidx;
};

void *histogram (void *threadarg) {
	int i, num;
	struct thread_data* arg = (struct thread_data*) threadarg;
	int startidx = arg->startidx;
	int maxidx = arg->maxidx;
	int temp[5]={0, 0, 0, 0, 0};
	//make local variables here instead of reading and writing to global memory each time???
	for (i=startidx; i<maxidx; i++) {
//		printf ("%d\n", input[startidx]);
		num = input_array[i];
		if (num > 0 && num <= BIN_MAX_NUM) {
			temp[0]+=1;
		}
		else if (num > BIN_MAX_NUM && num <= (BIN_MAX_NUM*2)) {
			temp[1]+=1;
		}
		else if (num > (BIN_MAX_NUM*2) && num <= (BIN_MAX_NUM*3)) {
			temp[2]+=1;
		}
		else if (num > (BIN_MAX_NUM*3) && num <= (BIN_MAX_NUM*4)) {
			temp[3]+=1;
		}
		else {
			temp[4]+=1;
		}
	}

	for (i=0; i<NUM_BINS; i++) {
		pthread_mutex_lock (&mutex);
		final_result[i] += temp[i];
		pthread_mutex_unlock (&mutex);	
	}

	pthread_exit(NULL);
}

int main () {

	int i, j;
	int main_result = 0;
	//create the thread variables
	pthread_t threads[NUM_ACCEL];
	struct thread_data data[NUM_ACCEL];

	//initialize structs to pass into accels
	for (i=0; i<NUM_ACCEL; i++) {
		data[i].startidx = i*OPS_PER_ACCEL;
		data[i].maxidx = (i+1)*OPS_PER_ACCEL;
	}

	//fork the threads
	for (i=0; i<NUM_ACCEL; i++) {
		pthread_create(&threads[i], NULL, histogram, (void *)&data[i]);
	}
	
	//join the threads
	for (i=0; i<NUM_ACCEL; i++) {
		pthread_join(threads[i], NULL);
	}

	//check the results	
	for (i=0; i<NUM_BINS; i++) {
		main_result += (final_result[i]==expected_array[i]);
	}

	//check final result
	printf ("Result: %d\n", main_result);
	if (main_result == NUM_BINS) {
		printf("RESULT: PASS\n");
	} else {
		printf("RESULT: FAIL\n");
	}
}
