#define NUM_ACCEL 4
#define ARRAY_SIZE 10000
#define OPS_PER_ACCEL ARRAY_SIZE/NUM_ACCEL

#include <stdio.h>
#include <pthread.h>
#include "add.h"

//int output[NUM_ACCEL];

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int final_result=0;

struct thread_data{
   int  startidx;
   int  maxidx;
};

void *add (void *threadarg)
{
	int result=0;
	int i;
	struct thread_data* arg = (struct thread_data*) threadarg;
	int startidx = arg->startidx;
	int maxidx = arg->maxidx;
	for (i=startidx; i<maxidx; i++)
	{
	  result += array[i];
	}
	//printf("startidx = %d, maxidx = %d, result = %d\n", startidx, maxidx, result);
	pthread_mutex_lock (&mutex);
	final_result += result;
	pthread_mutex_unlock (&mutex);
	pthread_exit(NULL);
}


int
main ()
{
	int i;
	//create the thread variables
	pthread_t threads[NUM_ACCEL];
	struct thread_data data[NUM_ACCEL];

	for (i=0; i<NUM_ACCEL; i++) {
		//initialize structs to pass into accels
		data[i].startidx = i*OPS_PER_ACCEL;
		data[i].maxidx = (i+1)*OPS_PER_ACCEL;
	}

	//launch threads
	for (i=0; i<NUM_ACCEL; i++) {
		pthread_create(&threads[i], NULL, add, (void *)&data[i]);
	}
	 
	//join the threads
	for (i=0; i<NUM_ACCEL; i++) {
		pthread_join(threads[i], NULL);
	}

	//check result
	printf ("Result: %d\n", final_result);
	if (final_result == 55000) {
		printf("RESULT: PASS\n");
	} else {
		printf("RESULT: FAIL\n");
	}

	return 0;
}
