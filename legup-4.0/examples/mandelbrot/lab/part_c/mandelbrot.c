// Mandelbrot kernel

// Author: Andrew Canis
// Date: July 1, 2012

#include <stdio.h>
#include <pthread.h>

#define DECIMAL_PLACES 28
#define int2fixed(num) ((num) << DECIMAL_PLACES)
#define fixedmul(a, b) ((((long long)a) * ((long long)b)) >> DECIMAL_PLACES)
#define fixed2int(num) ((num) >> DECIMAL_PLACES)

#define WIDTH 64
#define HEIGHT 64
#define MAX_ITER 50
#define NUM_ACCEL 4
#define OPS_PER_ACCEL HEIGHT/NUM_ACCEL
#define OMP_ACCEL 4

struct thread_data{
	int  startidx;
	int  maxidx;
};

volatile unsigned char img[WIDTH][HEIGHT];

void *mandelbrot(void *threadarg) {
	int i, j, tid;
	int count = 0;
	struct thread_data* arg = (struct thread_data*) threadarg;
	int startidx = arg->startidx;
	int maxidx = arg->maxidx;

	for (j = startidx; j < maxidx; j++) {
		for (i = 0; i < WIDTH; i++) {

			int x_0 = int2fixed(-2) + ((((3 << 20) * i/WIDTH) ) << 8);
			int y_0 = int2fixed(-1) + ((((2 << 20) * j/HEIGHT) ) << 8);

			int x = 0;
			int y = 0;
			int xtmp;
			unsigned char iter;
			unsigned char fiter = 0;

loop:		for (iter = 0; iter < MAX_ITER; iter++) {
				long long abs_squared = fixedmul(x,x) + fixedmul(y,y);

				xtmp = fixedmul(x,x) - fixedmul(y,y) + x_0;
				y = fixedmul(int2fixed(2), fixedmul(x,y)) + y_0;
				x = xtmp;

				fiter  +=  abs_squared <= int2fixed(4);
			}

			//get black or white
			unsigned char colour = (fiter >= MAX_ITER) ? 0 : 1;
			//accumulate colour
			count += colour;
			//update image
			img[i][j] = colour;
		}
	}

	pthread_exit((void*)count);
}


int main() {
	int final_result = 0;
	int i, j;
	int count[NUM_ACCEL];
	pthread_t threads[NUM_ACCEL];
	struct thread_data data[NUM_ACCEL];

	//initialize structs to pass into accels
	for (i=0; i<NUM_ACCEL; i++) {
		data[i].startidx = i*OPS_PER_ACCEL;
		if (i == NUM_ACCEL-1) {
			data[i].maxidx = HEIGHT;
		} else {
			data[i].maxidx = (i+1)*OPS_PER_ACCEL;
		}
	}

	//launch threads
	//for (i=0; i<1; i++) {
	for (i=0; i<NUM_ACCEL; i++) {
		pthread_create(&threads[i], NULL, mandelbrot, (void *)&data[i]);
	}

	//join the threads
	//for (i=0; i<1; i++) {
	for (i=0; i<NUM_ACCEL; i++) {
		pthread_join(threads[i], (void**)&count[i]);
	}

	//sum the results
	for (i=0; i<NUM_ACCEL; i++) {
		final_result += count[i];
	}
	//count = mandelbrot();
	printf("Count: %d\n", final_result);
	if (final_result == 2989) {
		printf("PASS\n");
	} else {
		printf("FAIL\n");
	}
}
