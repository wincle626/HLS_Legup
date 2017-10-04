#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#define ARRAY_SIZE 400

#include "test.h"

int main () 
{
	//START_COUNTER();
	int output[ARRAY_SIZE];
    int i, tid, result=0;
 	#pragma omp parallel for num_threads(8) private(i, tid)
	for (i = 0; i < ARRAY_SIZE; i++){
		tid = omp_get_thread_num();
		output[i] = A_array[i]*B_array[i];
		if (expected_output[i] != output[i]) {
			printf("Computation is wrong for output[%d] on Accel %d!\n", i, tid);
			printf("expected_output = [%d] = %d, output[%d] = %d\n", i, expected_output[i], i, output[i]);
		} else {
			printf("Computation is correct for output[%d] on Accel %d!\n", i, tid);
		}
	}

	//checking result on the processor to make sure
	for (i = 0; i < ARRAY_SIZE; i++){
		if (expected_output[i] == output[i]) {
			result++;	
		}
	}

    printf("Result: %d\n", result);
    if (result == ARRAY_SIZE) {
        printf("RESULT: PASS\n");
    } else {
        printf("RESULT: FAIL\n");
    }
	//STOP_COUNTER();
}

