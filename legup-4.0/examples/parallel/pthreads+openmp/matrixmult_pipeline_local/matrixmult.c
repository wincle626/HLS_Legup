/* Parallelizable Matrix Multiply Benchmark
 * 
 * do AxB=C
 * 
 * by Kevin Nam
 */
#include <stdio.h>
#include <pthread.h>

#define INPUTA_MATRIXDIMENSION_X 100
#define INPUTA_MATRIXDIMENSION_Y 6
#define NUM_ACCEL INPUTA_MATRIXDIMENSION_Y

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

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int final_result = 0;

struct thread_data{
   int  startidx;
   int  maxidx;
};
 
//int calc(int y, int j) {
void calc(int y, int j) {
	int i, result = 0;
	loop: for (i = 0; i < INPUTA_MATRIXDIMENSION_X; i++) {
		result+= matrixA[y][i]*matrixB[i][j];
	}
	matrixC[y][j] = result;
	//return result;
}

// Calculate the values for an entire row
//void multiply (int y, int INPUTA_MATRIXDIMENSION_X){
void *multiply (void *threadarg) {
	
	int i,j;
	int y = *((int *)threadarg);
//	int result = 0;
// 	#pragma omp parallel for num_threads(6) private(j, result)
 	#pragma omp parallel for num_threads(3) private(j)
	for (j = 0; j < INPUTB_MATRIXDIMENSION_X; j++){
		//calculating the multiplication
		//result += calc(y, j);
		calc(y, j);
//		loop: for (i = 0; i < INPUTA_MATRIXDIMENSION_X; i++) {
//			result+= matrixA[y][i]*matrixB[i][j];
//		}
		
//		matrixC[y][j] = result;
//		result = 0;
		//writing back the result
		//matrixC[y][j] = result;
		//result = 0;
	}
	pthread_exit(NULL);
}

int main(){

	int x = 0, y = 0;
	int main_result=0;
	//create the thread variables
	pthread_t threads[NUM_ACCEL];
 	int data[NUM_ACCEL];

	//prepare data
	for (y = 0; y<NUM_ACCEL; y++) {
		data[y]=y;
	}

	// Do the multiplies (row at a time)
	for (y = 0; y<NUM_ACCEL; y++) {
	//for (y = 0; y<1; y++) {
		pthread_create(&threads[y], NULL, multiply, (void *)&data[y]);
	}

	//join the threads
	for (y=0; y<NUM_ACCEL; y++) {
	//for (y=0; y<1; y++) {
		pthread_join(threads[y], NULL);
	}

	// check result
	for (y = 0; y < OUTPUT_MATRIXDIMENSION_Y; y++){
	//for (y = 0; y < 1; y++){
		for (x = 0; x < OUTPUT_MATRIXDIMENSION_X; x++){
			main_result += expected[y][x] == matrixC[y][x];
			printf("expected[%d][%d] = %d, accel[%d][%d] = %d\n", y, x, expected[y][x], y, x, matrixC[y][x]);
		}
	}

	//check final result
	printf ("Result: %d\n", main_result);
	if (main_result == OUTPUT_MATRIXDIMENSION_X*OUTPUT_MATRIXDIMENSION_Y) {
		printf("RESULT: PASS\n");
	} else {
		printf("RESULT: FAIL\n");
	}
	return main_result;
}

