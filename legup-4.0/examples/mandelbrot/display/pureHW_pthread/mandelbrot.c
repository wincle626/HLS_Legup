// Mandelbrot kernel
// Author: Andrew Canis
// Date: July 1, 2012

#include <stdio.h>
#include <pthread.h>

void __attribute__ ((noinline)) __attribute__ ((used)) 
	custom_VGA_display (unsigned int i, unsigned int j, unsigned char color) {
	printf ("Hello World : %d, %d, %c", i, j, color);
}

#define DECIMAL_PLACES 28
#define int2fixed(num) ((num) << DECIMAL_PLACES)
#define fixedmul(a, b) ((((long long)a) * ((long long)b)) >> DECIMAL_PLACES)
#define fixed2int(num) ((num) >> DECIMAL_PLACES)

//#define WIDTH 128
//#define HEIGHT 128
#define WIDTH 1024
#define HEIGHT 768
#define BYTES_PER_ROW WIDTH>>3

#define NUM_ACCEL 4
#define OPS_PER_ACCEL HEIGHT/NUM_ACCEL

#define CX -247586821 // = -0.92233278103<<DECIMAL_PLACES
#define CY   83284740 // =  0.31025983508<<DECIMAL_PLACES
#define MAX_ITER 50

struct thread_data{
   int  startidx;
   int  maxidx;
   int curr_h;
   int curr_w;
};

volatile unsigned char img[HEIGHT][BYTES_PER_ROW] = {0};

inline void write_pixel (unsigned int row, unsigned int column, unsigned char color) {
	unsigned char orig = img [row][column>>3];
	unsigned char bit_idx = column % 8;
	unsigned char new = ( orig & ( 0xFF - (1<<bit_idx) ) ) | (color<<bit_idx);
	img [row][column>>3] = new;
}

void copy_all_pixel () {
	int i, j, k;
	for (i=0; i<HEIGHT; i++) {
		for (j=0; j<BYTES_PER_ROW; j++) {
			unsigned char color = img [i][j];
			for (k=0; k<8; k++) {
				custom_VGA_display (i, (j<<3)+k, color&0x1);
				color >>= 1;
			}
		}
	}
}

void *mandelbrot(void *threadarg) {
	int i, j, tid;
	struct thread_data* arg = (struct thread_data*) threadarg;
	int startidx = arg->startidx;
	int maxidx = arg->maxidx;
	int curr_w = arg->curr_w;
	int curr_h = arg->curr_h;

	for (j = startidx; j < maxidx; j++) {
		for (i = 0; i < WIDTH; i++) {

			int x_0 = CX + ( ((i<<10)/WIDTH ) - (1<<9)/*0.5<<10*/ ) * curr_w;
			int y_0 = CY + ( ((j<<10)/HEIGHT) - (1<<9)            ) * curr_h;

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
			//update image
			write_pixel ( j, i, colour );
		}
	}

	pthread_exit(NULL);
}

void __attribute__ ((noinline)) __attribute__ ((used)) 
	thread_manager (int curr_w, int curr_h) {
	int i, j;
	pthread_t threads[NUM_ACCEL];
	struct thread_data data[NUM_ACCEL];

	//initialize structs to pass into accels
	for (i=0; i<NUM_ACCEL; i++) {
		data[i].startidx = i*OPS_PER_ACCEL;
		data[i].curr_w = curr_w;
		data[i].curr_h = curr_h;
		if (i == NUM_ACCEL-1) {
			data[i].maxidx = HEIGHT;
		} else {
			data[i].maxidx = (i+1)*OPS_PER_ACCEL;
		}
	}

	//launch threads
	for (i=0; i<NUM_ACCEL; i++) {
		pthread_create(&threads[i], NULL, mandelbrot, (void *)&data[i]);
	}

	//join the threads
	for (i=0; i<NUM_ACCEL; i++) {
		pthread_join(threads[i], NULL);
	}
	return;
}

int main() {
   	while (1) {
		int curr_w = 3 << 18;
		int curr_h = 2 << 18;
 		int zoom = 0;
  		for (zoom = 1; zoom < 60; zoom ++) {
			thread_manager (curr_w, curr_h);
			copy_all_pixel();
			curr_w = (curr_w>>1) + (curr_w>>2) + (curr_w>>3);
			curr_h = (curr_h>>1) + (curr_h>>2) + (curr_h>>3);
 		}
	}
	return 0;
}
