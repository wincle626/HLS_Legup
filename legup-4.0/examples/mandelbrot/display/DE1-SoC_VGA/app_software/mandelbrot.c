// Mandelbrot kernel
// Author: Andrew Canis
// Date: July 1, 2012

#include <stdio.h>

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

// 0->frame_a; 1->frame_b;
// start from displaying frame_a and writing to frame_b
char current_frame = 1;	
void init_frame_address () {
	unsigned int* backup_frame_address = (unsigned int *) 0xC2000004;
	// the sdram base address from the DMA's perspective is 0x0
	(* backup_frame_address) = 0x00180000;
}

void swap_frame () {
	int* swap_register = (int *) 0xC2000000;
	(* swap_register) = 1;
	current_frame = 1 - current_frame;
}

inline void write_pixel (unsigned int row, unsigned int column, unsigned char color) {
	// update image now.
	// the sdram addresses from ARM's perspective
	short * pixel_buffer = current_frame ? (short *)(0xC0180000) : (short *)(0xC0000000);
	pixel_buffer += column;
	pixel_buffer += row * 1024;
	(*pixel_buffer) = color ? 0xFFFF : 0x0000;
}

	

void mandelbrot() {
	int i, j, zoom;
	
	// initialize the frame
	init_frame_address();
	// start computing
	while (1) {
		for (zoom = 0; zoom < ZOOM_ITER*2-2; zoom ++) {
			for (i = 0; i < WIDTH; i++) {
				for (j = 0; j < HEIGHT; j++) {
					int x_0 = CX + ( ((i<<10)/WIDTH ) - (1<<9)/*0.5<<10*/ ) * w_scales[zoom];
					int y_0 = CY + ( ((j<<10)/HEIGHT) - (1<<9)            ) * h_scales[zoom];

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
					unsigned char color = (fiter >= MAX_ITER) ? 0 : 1;
					// update image
					write_pixel ( j, i, color );
				}
			}
			// finish computing a frame, swap the frame the address so it can be displayed
			swap_frame();
		}
	}
	return;
}

int main() {
	prepare_scales();
	mandelbrot();
	return 0;
}
