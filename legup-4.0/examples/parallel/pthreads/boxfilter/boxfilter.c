#define DIMENSION_X 1000
#define DIMENSION_Y 6

#include <stdio.h>
#include <pthread.h>
#include "boxfilter.h"


// The values of the 3x3 box to filter with.
/*
 *  1 2 3
 *  4 5 6
 *  7 8 9
 */
//Constant to filter by
#define BOX1 3
#define BOX2 3
#define BOX3 3
#define BOX4 3
#define BOX5 3
#define BOX6 3
#define BOX7 3
#define BOX8 3
#define BOX9 3

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int final_result=0;

/// a 3x3 box filter. Filter an entire row of the matrix in one call.
void *filter (void *threadarg){
	int x;
	int checksum =0;
	int y = *((int *)threadarg);
/*	
	if (y==2 && x==337) {
		printf("input address = %p\n", &input);
	}*/

	for (x = 0; x < DIMENSION_X; x++){
		int sum = 0;
		//ex if at the top left corner of the image, we can only average 
		//the (y,x),(y+1,x),(y+1,x+1),(y,x+1). therefore pixelsAveraged = 4
		// if we're not at an edge of the image, then pixels averaged is 3x3=9
		int pixelsAveraged = 1; 
		// Sum them up
		if (y-1 >= 0 && x-1 >= 0){ 
			sum+= input[y-1][x-1]*BOX1; //top left
			pixelsAveraged++;
			/*if (y==2 && x==337) {
				printf("top left, x = %d, offset = %d, value = %d\n", x, &input[y-1][x-1] - &input[0][0], input[y-1][x-1]);
			}*/
		}
		if (y-1 >= 0 && x+1 < DIMENSION_X){
			sum+= input[y-1][x+1]*BOX3; //top right
			pixelsAveraged++;
			/*if (y==2 && x==337) {
				printf("top right, x = %d, offset = %d, value = %d\n", x, &input[y-1][x+1] - &input[0][0], input[y-1][x+1]);
			}*/
		}
		if (y+1 < DIMENSION_Y && x+1 < DIMENSION_X){
			sum+= input[y+1][x+1]*BOX9; //bottom right
			pixelsAveraged++;
			/*if (y==2 && x==337) {
				printf("bottom right, x = %d, offset = %d, value = %d\n", x, &input[y+1][x+1] - &input[0][0], input[y+1][x+1]);
			}*/
		}
		if (y+1 < DIMENSION_Y && x-1 >= 0) {
			sum+= input[y+1][x-1]*BOX7; //bottom left
			pixelsAveraged++;
			/*if (y==2 && x==337) {
				printf("bottom left, x = %d, offset = %d, value = %d\n", x, &input[y+1][x-1] - &input[0][0], input[y+1][x-1]);
			}*/
		}
		if (y-1 >= 0) {
			sum+= input[y-1][x]*BOX2; //top
			pixelsAveraged++;
			/*if (y==2 && x==337) {
				printf("top, x = %d, offset = %d, value = %d\n", x, &input[y-1][x] - &input[0][0], input[y-1][x]);
			}*/
		}
		if (x-1 >= 0) {
			sum+= input[y][x-1]*BOX4; //left
			pixelsAveraged++;
			/*if (y==2 && x==337) {
				printf("left, x = %d, offset = %d, value = %d\n", x, &input[y][x-1] - &input[0][0], input[y][x-1]);
			}*/
		}
		if (y+1 < DIMENSION_Y) {
			sum+= input[y+1][x]*BOX8; //bottom
			pixelsAveraged++;
			/*if (y==2 && x==337) {
				printf("bottom, x = %d, offset = %d, value = %d\n", x, &input[y+1][x] - &input[0][0], input[y+1][x]);
			}*/
		}
		if (x+1 < DIMENSION_X) {
			sum+= input[y][x+1]*BOX6; //right
			pixelsAveraged++;
			/*if (y==2 && x==337) {
				printf("right, x = %d, offset = %d, value = %d\n", x, &input[y][x+1] - &input[0][0], input[y][x+1]);
			}*/
		}
		sum+=input[y][x]; //centre
		/*if (y==2 && x==337) {
			printf("centre, x = %d, offset = %d, value = %d\n", x, &input[y][x] - &input[0][0], input[y][x]);
			printf("x = %d, sum/pixelsAveraged = %d\n", x, sum/pixelsAveraged);
		}*/
		checksum += sum/pixelsAveraged;
	}
	//printf("y = %d, result = %d\n", y, checksum);
	pthread_mutex_lock (&mutex);
	final_result += checksum;
	pthread_mutex_unlock (&mutex);
	pthread_exit(NULL);
}

int main(){
	int y,x;
	pthread_t threads[DIMENSION_Y];
	int i[DIMENSION_Y];

	for (y = 0; y < DIMENSION_Y; y++) {
		//filter(y, original, checksum_output);
		i[y] = y;
		pthread_create(&threads[y], NULL, filter, (void *)&i[y]);
	}

	//join the threads
	for (y = 0; y < DIMENSION_Y; y++) {
		pthread_join(threads[y], NULL);
	}

	printf ("Result: %d\n", final_result);
	if (final_result == 87798) {
		printf("RESULT: PASS\n");
	} else {
		printf("RESULT: FAIL\n");
	}
}
