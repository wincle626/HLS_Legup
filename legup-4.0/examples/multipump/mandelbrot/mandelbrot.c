// Mandelbrot kernel
// Author: Andrew Canis
// Date: July 1, 2012

#include <stdio.h>

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

#define PRINT_IMG 1

#define DECIMAL_PLACES 28
#define int2fixed(num) ((num) << DECIMAL_PLACES)
#define fixedmul(a, b) ((((long long)a) * ((long long)b)) >> DECIMAL_PLACES)
#define fixed2int(num) ((num) >> DECIMAL_PLACES)

#define WIDTH 32
#define HEIGHT 32
//#define MAX_ITER 10
//#define MAX_ITER 2
#define MAX_ITER 10

int main() {
	int i, j;
    unsigned char img[WIDTH][HEIGHT];
    unsigned char img1[WIDTH][HEIGHT];
    unsigned char img2[WIDTH][HEIGHT];
    unsigned char img3[WIDTH][HEIGHT];
    int count = 0;

	for (j = 0; j < HEIGHT; j++) {
		//for (i = 0; i < WIDTH; i++) {
		//for (i = 0; i < WIDTH; i+=2) {
		for (i = 0; i < WIDTH; i+=4) {

			int x_0 = int2fixed(-2) + ((((3 << 20) * i) >> 5 ) << 8);
			int y_0 = int2fixed(-1) + ((((2 << 20) * j) >> 5 ) << 8);

			int x_1 = int2fixed(-2) + ((((3 << 20) * (i+1)) >> 5 ) << 8);
			int x_2 = int2fixed(-2) + ((((3 << 20) * (i+2)) >> 5 ) << 8);
			int x_3 = int2fixed(-2) + ((((3 << 20) * (i+3)) >> 5 ) << 8);

			int x = 0;
			int y = 0;
			int xtmp;
			unsigned char iter;
			unsigned char fiter = 0;

			int x2 = 0;
			int y2 = 0;
			int xtmp2;
			unsigned char fiter2 = 0;

			int x3 = 0;
			int y3 = 0;
			int xtmp3;
			unsigned char fiter3 = 0;

			int x4 = 0;
			int y4 = 0;
			int xtmp4;
			unsigned char fiter4 = 0;

            for (iter = 0; iter < MAX_ITER; iter++) {
                long long abs_squared = fixedmul(x,x) + fixedmul(y,y);
                long long abs_squared2 = fixedmul(x2,x2) + fixedmul(y2,y2);
                long long abs_squared3 = fixedmul(x3,x3) + fixedmul(y3,y3);
                long long abs_squared4 = fixedmul(x4,x4) + fixedmul(y4,y4);

				xtmp = fixedmul(x,x) - fixedmul(y,y) + x_0;
				y = fixedmul(int2fixed(2), fixedmul(x,y)) + y_0;
				x = xtmp;

				xtmp2 = fixedmul(x2,x2) - fixedmul(y2,y2) + x_1;
				y2 = fixedmul(int2fixed(2), fixedmul(x2,y2)) + y_0;
				x2 = xtmp2;

				xtmp3 = fixedmul(x3,x3) - fixedmul(y3,y3) + x_2;
				y3 = fixedmul(int2fixed(2), fixedmul(x3,y3)) + y_0;
				x3 = xtmp3;

				xtmp4 = fixedmul(x4,x4) - fixedmul(y4,y4) + x_3;
				y4 = fixedmul(int2fixed(2), fixedmul(x4,y4)) + y_0;
				x4 = xtmp4;

                fiter  +=  abs_squared <= int2fixed(4);
                fiter2 += abs_squared2 <= int2fixed(4);
                fiter3 += abs_squared3 <= int2fixed(4);
                fiter4 += abs_squared4 <= int2fixed(4);

                //int f1 = !fiter &&  abs_squared > int2fixed(4);
                //int f2 = !fiter2 && abs_squared2 > int2fixed(4);
                //int f3 = !fiter3 && abs_squared3 > int2fixed(4);
                //int f4 = !fiter4 && abs_squared4 > int2fixed(4);
                //fiter = (f1) ? iter : fiter;
                //fiter2 = (f2) ? iter : fiter2;
                //fiter3 = (f3) ? iter : fiter3;
                //fiter4 = (f4) ? iter : fiter4;


			}

            //int m1 = !fiter && iter == MAX_ITER;
            //fiter = (m1) ? MAX_ITER : fiter;
            //int m2 = !fiter2 && iter == MAX_ITER;
            //fiter2 = (m2) ? MAX_ITER : fiter2;
            //int m3 = !fiter3 && iter == MAX_ITER;
            //fiter3 = (m3) ? MAX_ITER : fiter3;
            //int m4 = !fiter4 && iter == MAX_ITER;
            //fiter4 = (m4) ? MAX_ITER : fiter4;

            //unsigned colour = (iter > MAX_ITER) ? MAX_ITER : iter;
            //unsigned char colour = iter;
            //unsigned char colour = fiter;
            unsigned char colour = (fiter >= MAX_ITER) ? 0 : 1;
            count += colour;
			img[i][j] = colour;

            //colour = fiter2;
            colour = (fiter2 >= MAX_ITER) ? 0 : 1;
            count += colour;
			img1[i+1][j] = colour;

            //colour = fiter3;
            count += colour;
            colour = (fiter3 >= MAX_ITER) ? 0 : 1;
			img2[i+2][j] = colour;

            //olour = fiter4;
            count += colour;
            colour = (fiter4 >= MAX_ITER) ? 0 : 1;
			img3[i+3][j] = colour;

		}
	}

#ifdef PRINT_IMG

    unsigned char final[WIDTH][HEIGHT];
	for (j = 0; j < HEIGHT; j++) {
		for (i = 0; i < WIDTH; i+=4) {
			final[i][j] = img[i][j];
			final[i+1][j] = img1[i+1][j];
			final[i+2][j] = img2[i+2][j];
			final[i+3][j] = img3[i+3][j];
        }
    }
    print_image(WIDTH, HEIGHT, 1, final);
    //print_image(WIDTH, HEIGHT, 1, img);
    //print_image(WIDTH, HEIGHT, MAX_ITER, img);

#else
    printf("Count: %d\n", count);
    if (count == 656) {
        printf(" _____         _____ _____ ______ _____  \n");
        printf("|  __ \\ /\\    / ____/ ____|  ____|  __ \\ \n");
        printf("| |__) /  \\  | (___| (___ | |__  | |  | |\n");
        printf("|  ___/ /\\ \\  \\___ \\\\___ \\|  __| | |  | |\n");
        printf("| |  / ____ \\ ____) |___) | |____| |__| |\n");
        printf("|_| /_/    \\_\\_____/_____/|______|_____/ \n");
    } else {
        printf(" ______      _____ _      ______ _____  \n");
        printf("|  ____/\\   |_   _| |    |  ____|  __ \\ \n");
        printf("| |__ /  \\    | | | |    | |__  | |  | |\n");
        printf("|  __/ /\\ \\   | | | |    |  __| | |  | |\n");
        printf("| | / ____ \\ _| |_| |____| |____| |__| |\n");
        printf("|_|/_/    \\_\\_____|______|______|_____/ \n");
    }
#endif

	return count;
}
