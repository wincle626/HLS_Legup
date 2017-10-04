/* Parallelizable Matrix Multiply Benchmark
 * 
 * do AxB=C
 * 
 * by Kevin Nam
 */
#include <omp.h>
#include <stdio.h>

#define INPUTA_MATRIXDIMENSION_X 100
#define INPUTA_MATRIXDIMENSION_Y 6

#define INPUTB_MATRIXDIMENSION_X INPUTA_MATRIXDIMENSION_Y
#define INPUTB_MATRIXDIMENSION_Y INPUTA_MATRIXDIMENSION_X

#define OUTPUT_MATRIXDIMENSION_X INPUTB_MATRIXDIMENSION_X
#define OUTPUT_MATRIXDIMENSION_Y INPUTA_MATRIXDIMENSION_Y


int matrixA [INPUTA_MATRIXDIMENSION_Y][INPUTA_MATRIXDIMENSION_X] = {  
{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10}
};

// matrix B  [y][x]
int matrixB [INPUTB_MATRIXDIMENSION_Y][INPUTB_MATRIXDIMENSION_X] = {  
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6},
{1, 2, 3, 4, 5, 6}
};

												
// matrix C  [y][x]
int matrixC [OUTPUT_MATRIXDIMENSION_Y][OUTPUT_MATRIXDIMENSION_X];

// expected output 
int expected[OUTPUT_MATRIXDIMENSION_Y][OUTPUT_MATRIXDIMENSION_X] = { 	
{550, 1100, 1650, 2200, 2750, 3300},
{550, 1100, 1650, 2200, 2750, 3300},
{550, 1100, 1650, 2200, 2750, 3300},
{550, 1100, 1650, 2200, 2750, 3300},
{550, 1100, 1650, 2200, 2750, 3300},
{550, 1100, 1650, 2200, 2750, 3300}
};

// Calculate the values for an entire row
void multiply (int y, int a_matrix_dim_x){
	int i,j;
	int result = 0;
	for (j = 0; j < INPUTB_MATRIXDIMENSION_X; j++){
		//calculating the multiplication
		loop: for (i = 0; i < a_matrix_dim_x; i++) {
			result+= matrixA[y][i]*matrixB[i][j];
		}

		//writing back the result
		matrixC[y][j] = result;
		result = 0;
	}
}

int main(){

	int x = 0, y = 0;

	// Do the multiplies (row at a time)
 	#pragma omp parallel for num_threads(6) private(y)
	for (y = 0; y < INPUTA_MATRIXDIMENSION_Y; y++) {
		multiply(y, INPUTA_MATRIXDIMENSION_X);
	}

	// check result
	int result = 0;
	for (y = 0; y < OUTPUT_MATRIXDIMENSION_Y; y++){
		for (x = 0; x < OUTPUT_MATRIXDIMENSION_X; x++){
			result += expected[y][x] == matrixC[y][x];
		}
	}

	//check final result
	printf ("Result: %d\n", result);
	if (result == OUTPUT_MATRIXDIMENSION_X*OUTPUT_MATRIXDIMENSION_Y) {
		printf("RESULT: PASS\n");
	} else {
		printf("RESULT: FAIL\n");
	}
	return result;
}
