//Two parallel functions for this benchmark: 1. swap, which transposes the matrix, 2. check, which checks the output matrix with the expected. 

#define INPUT_ROWS 1024
#define INPUT_COLS 6
#define NUM_ACCEL INPUT_COLS

#define OUTPUT_ROWS INPUT_COLS
#define OUTPUT_COLS INPUT_ROWS

#include <stdio.h>
#include <pthread.h>
#include "matrixtrans.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int final_result = 0;

void *swap (void *threadarg) {
	int y;
	int checksum=0;
	int accel_num = *((int *)threadarg);
	int output[INPUT_ROWS];
//	for (y=0; y<INPUT_ROWS; y++) {
//		output [y] = input_array[y][accel_num];		
//	}

	for (y=0; y<INPUT_ROWS; y++) {
		checksum += (expected_array[accel_num][y] == input_array[y][accel_num]);		
	}

	pthread_mutex_lock (&mutex);
	final_result += checksum;
	pthread_mutex_unlock (&mutex);

	pthread_exit(NULL);
}


int main() {

	int i;
	int main_result=0;
	//create the thread variables
	pthread_t threads[NUM_ACCEL];
 	int data[NUM_ACCEL];

	//Perform the Matrix Transpose
	for (i=0; i<NUM_ACCEL; i++) {
		data[i]=i;
		pthread_create(&threads[i], NULL, swap, (void *)&data[i]);
	}

	//join the threads
	for (i=0; i<NUM_ACCEL; i++) {
		pthread_join(threads[i], NULL);
	}

	//check final result
	printf ("Result: %d\n", final_result);
	if (final_result == 6144) {
		printf("RESULT: PASS\n");
	} else {
		printf("RESULT: FAIL\n");
	}
}
