// Mandelbrot kernel
// Author: Andrew Canis
// Date: July 1, 2012

#include <stdio.h>

void __attribute__ ((noinline)) __attribute__ ((used)) 
	custom_VGA_display (unsigned int i, unsigned int j, unsigned char color) {
	printf ("Hello World : %d, %d, %c", i, j, color);
}

#define DECIMAL_PLACES 28
#define int2fixed(num) ((num) << DECIMAL_PLACES)
#define fixedmul(a, b) ((((long long)a) * ((long long)b)) >> DECIMAL_PLACES)
#define fixed2int(num) ((num) >> DECIMAL_PLACES)

#define WIDTH 1024
#define HEIGHT 768
#define BYTES_PER_ROW WIDTH>>3

#define CX -247586821 // = -0.92233278103<<DECIMAL_PLACES
#define CY   83284740 // =  0.31025983508<<DECIMAL_PLACES
#define MAX_ITER 50
#define ZOOM_ITER 60

unsigned char img[HEIGHT][BYTES_PER_ROW] = {0};
unsigned int w_scales [ZOOM_ITER *2 -2] = {0};
unsigned int h_scales [ZOOM_ITER *2 -2] = {0};

void prepare_scales () {
	int i;
	int curr_w = 3<<18;
	int curr_h = 2<<18;
	// the initial frame size
	w_scales [0] = curr_w;
	h_scales [0] = curr_h;
	// the zooming frame sizes
	for (i = 1; i < ZOOM_ITER-1; i++) {
		curr_w = (curr_w>>1) + (curr_w>>2) + (curr_w>>3);
		curr_h = (curr_h>>1) + (curr_h>>2) + (curr_h>>3);
		w_scales [i] = curr_w;
		h_scales [i] = curr_h;
		w_scales [ZOOM_ITER *2 - 2 - i] = curr_w;
		h_scales [ZOOM_ITER *2 - 2 - i] = curr_h;
	}
	// the last frame size (smallest)
	curr_w = (curr_w>>1) + (curr_w>>2) + (curr_w>>3);
	curr_h = (curr_h>>1) + (curr_h>>2) + (curr_h>>3);
	w_scales [ZOOM_ITER-1] = curr_w;
	h_scales [ZOOM_ITER-1] = curr_h;

	return;
}
		

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

void mandelbrot() {
	int i, j, zoom;

	int x_0 [WIDTH] = {0};
	int y_0 [WIDTH] = {0};
	int x [WIDTH] = {0};
	int y [WIDTH] = {0};
	unsigned char fiter[WIDTH] = {0};

	while (1) {
		for (zoom = 0; zoom < ZOOM_ITER*2-2; zoom ++) {
			for (j = 0; j < HEIGHT; j++) {

lp1:			for (i = 0; i < WIDTH; i++) {
					// find x_0 and y_0 in fixed point by interpolation
					// Mandelbrot x-range [-2:1] -- "width" is 3
					// y-range [-1:1] -- "width" is 2
					x_0[i] = CX + ( ((i<<10)/WIDTH ) - (1<<9)/*0.5<<10*/ ) * w_scales[zoom];
					y_0[i] = CY + ( ((j<<10)/HEIGHT) - (1<<9)            ) * h_scales[zoom];
					x[i]=0;
					y[i]=0;
					fiter[i]=0;
				}

				int xtmp;
				unsigned char iter;
				for (iter = 0; iter < MAX_ITER; iter++) {
lp2:				for (i=0; i < WIDTH; i++) {
						long long abs_squared = fixedmul(x[i],x[i]) + fixedmul(y[i],y[i]);
						xtmp = fixedmul(x[i],x[i]) - fixedmul(y[i],y[i]) + x_0[i];
						y[i] = fixedmul(int2fixed(2), fixedmul(x[i],y[i])) + y_0[i];
						x[i] = xtmp;

						fiter[i]  +=  abs_squared <= int2fixed(4);
					}
				}

lp3:			for (i=0; i < WIDTH; i++) {
					//get black or white 
					unsigned char color = (fiter[i] >= MAX_ITER) ? 0 : 1;
					// update image
					write_pixel ( j, i, color );
				}
			}
			copy_all_pixel();
		}
	}
	return;
}


int main() {
	prepare_scales();
	mandelbrot();
	return 0;
}
