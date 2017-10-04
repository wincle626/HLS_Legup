#define NUM_ACCEL 4
#define ARRAY_SIZE 20
#define OPS_PER_ACCEL ARRAY_SIZE/NUM_ACCEL

#include <stdio.h>
#include <pthread.h>

int input[ARRAY_SIZE] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_barrier_t barr;

int final_result=0;

void *add ()
//void *add (void *threadarg)
{
	int result=0;
	int i;
	for (i=0; i<ARRAY_SIZE; i++)
	{
		pthread_mutex_lock (&mutex);
		final_result += input[i];
		//result += input[i];
		pthread_mutex_unlock (&mutex);
	}
	pthread_barrier_wait(&barr);
	result = final_result;
	pthread_exit((void*)result);
//  either pthread_exit or return works in this case
//	return ((void*)result);
}


int
main ()
{
	int main_result=0;
	int i;
	//create the thread variables
	pthread_t threads[NUM_ACCEL];
	//initialize barrier with the number of threads
	pthread_barrier_init(&barr, NULL, NUM_ACCEL);
	int result[NUM_ACCEL] = {0, 0, 0, 0};

	for (i=0; i<NUM_ACCEL; i++) {
		pthread_create(&threads[i], NULL, add, NULL);
	}
	 
	//join the threads
	for (i=0; i<NUM_ACCEL; i++) {
		pthread_join(threads[i], (void**)&result[i]);
		//pthread_join(threads[i], NULL);
	}

/*
	for (i=0; i<NUM_ACCEL; i++) {
		pthread_create(&threads[i], NULL, add, NULL);
	}
	 
	//join the threads
	for (i=0; i<NUM_ACCEL; i++) {
		pthread_join(threads[i], (void**)&result[i]);
		//pthread_join(threads[i], NULL);
	}

*/
	//check result
	for (i=0; i<NUM_ACCEL; i++) {
		printf("result[%d] = %d\n", i, result[i]);
		main_result += (result[i] == 840);
	}

	printf ("Result: %d\n", main_result);
	if (main_result == NUM_ACCEL) {
		printf("RESULT: PASS\n");
	} else {
		printf("RESULT: FAIL\n");
	}

	return 0;
}
