// Mandelbrot kernel
// Author: Andrew Canis
// Date: July 1, 2012

#include <stdio.h>
#include <omp.h>
inline void print_image(int width, int height, int max, unsigned char img[width][height]) {
    int i, j;
    printf("P2\n%d %d\n%d\n", width, height, max);
    for (j = 0; j < height; j++) {
        for (i = 0; i < width; i++) {
            // assume grayscale image
            printf("%d ", img[i][j]);
        }
        printf("\n");
    }
}

//#define PRINT_IMG 1 

#define DECIMAL_PLACES 28
#define int2fixed(num) ((num) << DECIMAL_PLACES)
#define fixedmul(a, b) ((((long long)a) * ((long long)b)) >> DECIMAL_PLACES)
#define fixed2int(num) ((num) >> DECIMAL_PLACES)

#define WIDTH 128
#define HEIGHT 128
//#define MAX_ITER 10
//#define MAX_ITER 2
#define MAX_ITER 50
#define NUM_ACCELS 4

volatile unsigned char img[WIDTH][HEIGHT];

int mandelbrot() {
	int i, j, tid;
    int count = 0;
    int temp[NUM_ACCELS] = {0};

    #pragma omp parallel for num_threads(NUM_ACCELS) private(i, j, tid)
	for (j = 0; j < HEIGHT; j++) {
		for (i = 0; i < WIDTH; i++) {

            tid = omp_get_thread_num();
			int x_0 = int2fixed(-2) + ((((3 << 20) * i/WIDTH) ) << 8);
			int y_0 = int2fixed(-1) + ((((2 << 20) * j/HEIGHT) ) << 8);

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
            //count += colour;
            temp[tid] += colour;
			//update image
			img[i][j] = colour;
		}
	}

    for (i=0; i<NUM_ACCELS; i++) {
        count += temp[i];
    }

	return count;
}


int main() {
	int i, j;
	int count=0;

	count = mandelbrot();
#ifdef PRINT_IMG
	int i, j;
    unsigned char final[WIDTH][HEIGHT];
	for (j = 0; j < HEIGHT; j++) {
		for (i = 0; i < WIDTH; i++) {
			final[i][j] = img[i][j];
/*			final[i+1][j] = img1[i+1][j];
			final[i+2][j] = img2[i+2][j];
			final[i+3][j] = img3[i+3][j];*/
        }
    }
    print_image(WIDTH, HEIGHT, 1, final);
    //print_image(WIDTH, HEIGHT, 1, img);
    //print_image(WIDTH, HEIGHT, MAX_ITER, img);

#else
    printf("Count: %d\n", count);
    if (count == 12013) {
        printf("PASS\n");
    } else {
        printf("FAIL\n");
    }
#endif

	return count;
}
