// Mandelbrot kernel -- reference implementation

// Author: Andrew Canis
// Date: July 1, 2012

// Revised for HK Summer Course, July 2014
// Performance-Aware Computing for Application Accelerators

#include <stdio.h>

#define DECIMAL_PLACES 28
#define int2fixed(num) ((num) << DECIMAL_PLACES)
#define fixedmul(a, b) ((((long long)a) * ((long long)b)) >> DECIMAL_PLACES)
#define fixed2int(num) ((num) >> DECIMAL_PLACES)

#define WIDTH 64
#define HEIGHT 64
#define MAX_ITER 50

int mandelbrot() {
	int i, j;
	int count = 0;

	for (j = 0; j < HEIGHT; j++) {
		for (i = 0; i < WIDTH; i++) {

			// find x_0 and y_0 in fixed point by interpolation
			// Mandelbrot x-range [-2:1] -- "width" is 3
			// y-range [-1:1] -- "width" is 2
			int x_0 = int2fixed(-2) + (((((3 << 20) * i)/WIDTH) ) << 8);
			int y_0 = int2fixed(-1) + (((((2 << 20) * j)/HEIGHT) ) << 8);

			int x = 0;
			int y = 0;
			int xtmp;
			unsigned char iter;
			unsigned char fiter = 0;

			for (iter = 0; iter < MAX_ITER; iter++) {
				long long abs_squared = fixedmul(x,x) + fixedmul(y,y);
				xtmp = fixedmul(x,x) - fixedmul(y,y) + x_0;
				y = fixedmul(int2fixed(2), fixedmul(x,y)) + y_0;
				x = xtmp;

				fiter  +=  abs_squared <= int2fixed(4);
			}

			//get black or white 
			unsigned char colour = (fiter >= MAX_ITER) ? 0 : 1;
			//accumulate colour
			count += colour; // we count the # of pixels that "escaped" (NOT in Mandelbrot set)
		}
	}

	return count;
}

int main() {

	int count;
	count = mandelbrot();

	// correctness checking
	printf("Count: %d\n", count);
	if (count == 2989) {
		printf("PASS\n");
	} else {
		printf("FAIL\n");
	}

	return count;
}


