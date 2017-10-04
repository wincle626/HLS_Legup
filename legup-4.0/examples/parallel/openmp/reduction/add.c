#define NUM_ACCEL 4
#define ARRAY_SIZE 10000
#define OPS_PER_ACCEL ARRAY_SIZE/NUM_ACCEL

#include <stdio.h>
#include "add.h"


int add ()
{
	int result=0;
	int i;
	#pragma omp parallel for num_threads(NUM_ACCEL) private(i) reduction(+ : result)
	for (i=0; i<ARRAY_SIZE; i++)
	{
	    result += array[i];
	}
    return result;
}


int
main ()
{
	int i, result1, result2;

    result1 = add();
    
	//check result
	printf ("Result: %d\n", result1);
	if (result1 == 55000) {
		printf("RESULT: PASS\n");
	} else {
		printf("RESULT: FAIL\n");
	}

	return 0;
}
