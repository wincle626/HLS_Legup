//Two parallel functions for this benchmark: 1. swap, which transposes the matrix, 2. check, which checks the output matrix with the expected. 

#define INPUT_ROWS 1024
#define INPUT_COLS 6
#define ACCEL_NUM INPUT_COLS

#define OUTPUT_ROWS INPUT_COLS
#define OUTPUT_COLS INPUT_ROWS

#include <stdio.h>
#include "matrixtrans.h"

void swap (int input[][INPUT_COLS], int expected[][OUTPUT_COLS], int input_y_dim, int accel_num) {
	int y;
	int checksum=0;
	int output [INPUT_ROWS];
//	for (y=0; y<input_y_dim; y++) {
//		output[y] = input[y][accel_num];		
//	}

	for (y=0; y<input_y_dim; y++) {
		checksum += (expected[accel_num][y] == input[y][accel_num]);		
	}

	result[accel_num] = checksum;
}


int main() {

	int i;
	int main_result=0;

	//Perform the Matrix Transpose
	#pragma omp parallel for num_threads(ACCEL_NUM) private(i)
	for (i=0; i<ACCEL_NUM; i++) {
		swap(input_array, expected_array, INPUT_ROWS, i);
	}

	//check result	
	for (i=0; i<ACCEL_NUM; i++) {
		main_result += result[i];
	}

	//check final result
	printf ("Result: %d\n", main_result);
	if (main_result == 6144) {
		printf("RESULT: PASS\n");
	} else {
		printf("RESULT: FAIL\n");
	}
}
