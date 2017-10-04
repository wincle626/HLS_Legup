#include <stdio.h>
#include <stdlib.h>

#define HEIGHT 512
#define WIDTH 512
#define DATASIZE HEIGHT*WIDTH

#include "input.h"
#include "golden.h"


/* 3x3 GX Sobel mask.  Ref: www.cee.hw.ac.uk/hipr/html/sobel.html */
const int Gx[3][3] = {	-1,  0,  1,
					-2,  0,  2,
					-1,  0,  1};

/* 3x3 GY Sobel mask.  Ref: www.cee.hw.ac.uk/hipr/html/sobel.html */
const int Gy[3][3] = {	1,  2,  1,
					0,  0,  0,
					-1, -2, -1};

int bound(int x) {
	// will involve control flow because LLVM will try
	// to avoid the second comparison if possible
	return (x > 255) ? 255 : ((x < 0) ? 0 : x);
}

#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
#define min( a, b ) ( ((a) < (b)) ? (a) : (b) )
int bound2(int x) {
	// no control flow
	return max(0, min(255, x));
}

int not_in_bounds(int x, int y) {
	return !(y > 0 && (y < (HEIGHT-1)) && (x > 0) && (x < (WIDTH-1)));
}

unsigned char output_image[HEIGHT][WIDTH] = {0};

void sobel(unsigned char input_image[HEIGHT][WIDTH], unsigned char
		output_image[HEIGHT][WIDTH]) {
	int x, y, xOffset, yOffset, x_dir, y_dir, pixel, edge_weight;

	for (y = 0; y < HEIGHT; y++) {
		for (x = 0; x < WIDTH; x++) {
			if (not_in_bounds(x, y)) continue;
			x_dir = 0; y_dir = 0;
			for(yOffset = -1; yOffset <= 1; yOffset++) {
				for(xOffset = -1; xOffset <= 1; xOffset++) {
					pixel = input_image[y+yOffset][x+xOffset];
					//printf("%d ", pixel);
					x_dir += pixel * Gx[1+xOffset][1+yOffset];
					y_dir += pixel * Gy[1+xOffset][1+yOffset];
				}
				//printf("\n");
			}
			edge_weight = bound(x_dir) + bound(y_dir);
			//printf("y=%d ; x=%d --> edge_weight = %d\n", y, x, edge_weight);
			output_image[y][x] = 255 - edge_weight;
			if (1) printf("y=%d;x=%d-->SUM=%d+%d=%d--outdata[%d]=%d\n", y, x,
					x_dir, y_dir,
					edge_weight, x + y*WIDTH, 
					(int)output_image[y][x]);
		}
	}
}

// allow us to pipeline by flattening the loop and getting rid of the bounds
// check also we need to manually if convert the bounds checking on x_dir and
// y_dir. II = 4 due to memory ports 8 loads from the image (8/2 = 4)
void sobel_pipelined(unsigned char input_image[HEIGHT][WIDTH], unsigned char
		output_image[HEIGHT][WIDTH]) {
	int i, x, y, xOffset, yOffset, x_dir, y_dir, pixel, edge_weight;

	x=0; y=1;
loop: for (i = 0; i < (HEIGHT-2)*(WIDTH-2); i++) {
		  y = (x == (WIDTH-2)) ? y + 1 : y;
		  x = (x == (WIDTH-2)) ? 1 : x + 1;

		  x_dir = 0; y_dir = 0;
		  for(yOffset = -1; yOffset <= 1; yOffset++) {
			  for(xOffset = -1; xOffset <= 1; xOffset++) {
				  pixel = input_image[y+yOffset][x+xOffset];
				  //printf("%d ", pixel);
				  x_dir += pixel * Gx[1+xOffset][1+yOffset];
				  y_dir += pixel * Gy[1+xOffset][1+yOffset];
			  }
			  //printf("\n");
		  }

		  edge_weight = bound2(x_dir) + bound2(y_dir);

		  //printf("y=%d ; x=%d --> edge_weight = %d\n", y, x, edge_weight);
		  output_image[y][x] = 255 - edge_weight;
		  if (1) printf("y=%d;x=%d-->SUM=%d+%d=%d--outdata[%d]=%d\n", y, x,
				  x_dir, y_dir,
				  edge_weight, x + y*WIDTH, 
				  (int)output_image[y][x]);
	  }
}



// shift registers
unsigned char prev_row[WIDTH] = {0};
int prev_row_index = 0;
unsigned char prev_prev_row[WIDTH] = {0};
int prev_prev_row_index = 0;

unsigned char window[3][3] = {0};

void print_window() {
	printf(	"%d %d %d\n"
			"%d %d %d\n"
			"%d %d %d\n",
			window[0][0], window[0][1], window[0][2],
			window[1][0], window[1][1], window[1][2],
			window[2][0], window[2][1], window[2][2]);
}

// use windowing and a shift register to reduce memory accesses to input_image
// to one per iteration
void sobel_fast(unsigned char input_image[HEIGHT][WIDTH], unsigned char
		output_image[HEIGHT][WIDTH]) {
	int x, y, xOffset, yOffset, x_dir, y_dir, pixel, edge_weight, x_offset, y_offset;

	int start = 0;
	for (y = 0; y < HEIGHT; y++) {
		for (x = 0; x < WIDTH; x++) {
		  if (1) printf("y=%d;x=%d\n", y, x);

			pixel = input_image[y][x];

			// window buffer:
			// 		window[0][0], window[0][1], window[0][2],
			// 		window[1][0], window[1][1], window[1][2],
			// 		window[2][0], window[2][1], window[2][2]

			// shift existing window to the left by one
			window[0][0] = window[0][1]; window[0][1] = window[0][2];
			window[1][0] = window[1][1]; window[1][1] = window[1][2];
			window[2][0] = window[2][1]; window[2][1] = window[2][2];

			// grab next column (the rightmost column of the sliding window)
			window[0][2] = prev_prev_row[prev_prev_row_index];
			window[1][2] = prev_row[prev_row_index];
			window[2][2] = pixel;

			print_window();

			// shift in new pixel
			prev_prev_row[prev_prev_row_index] = prev_row[prev_prev_row_index];
			prev_row[prev_row_index] = pixel;

			// adjust shift register indices
			prev_row_index = (prev_row_index+1)%WIDTH;
			prev_prev_row_index = (prev_prev_row_index+1)%WIDTH;

			// we only want to start calculating the value when
			// the shift registers are full and the window is valid
			if (!start) {
				start = (y == 2) && (x == 2);
				x_offset = 1;
				y_offset = 1;
			}
			if (!start) continue;


			if (not_in_bounds(x_offset, y_offset)) {
				x_offset++;
				continue;
			}

			x_dir = 0; y_dir = 0;
			for(yOffset = -1; yOffset <= 1; yOffset++) {
				for(xOffset = -1; xOffset <= 1; xOffset++) {
					//printf("%d ", window[1+yOffset][1+xOffset]);
					x_dir += window[1+yOffset][1+xOffset] * Gx[1+xOffset][1+yOffset];
					y_dir += window[1+yOffset][1+xOffset] * Gy[1+xOffset][1+yOffset];
				}
				//printf("\n");
			}
			
			edge_weight = bound(x_dir) + bound(y_dir);
			output_image[y_offset][x_offset] = 255 - edge_weight;
			if (1) printf("y=%d;x=%d-->SUM=%d+%d=%d--outdata[%d]=%d\n", y_offset, x_offset,
					x_dir, y_dir,
					edge_weight, x_offset + y_offset*WIDTH, 
					(int)output_image[y_offset][x_offset]);
			x_offset++;

		}
		y_offset++;
		x_offset = -1;
	}

}

// apply manual if-conversion and loop flattening to allow pipelining
// II = 1
void sobel_fast_pipelined(unsigned char input_image[HEIGHT][WIDTH], unsigned
		char output_image[HEIGHT][WIDTH]) {
	int x, y, xOffset, yOffset, x_dir, y_dir, pixel, edge_weight, i, x_offset, y_offset;

	x_offset = -1;
	y_offset = -1;
	int start = 0;
	x=-1; y=0;
loop: for (i = 0; i < (HEIGHT)*(WIDTH); i++) {
		  y = (x == (WIDTH-1)) ? y + 1 : y;
		  x = (x == (WIDTH-1)) ? 0 : x + 1;
		  if (0) printf("y=%d;x=%d\n", y, x);

		  pixel = input_image[y][x];

		  // window buffer:
		  // 		window[0][0], window[0][1], window[0][2],
		  // 		window[1][0], window[1][1], window[1][2],
		  // 		window[2][0], window[2][1], window[2][2]

		  // shift existing window to the left by one
		  window[0][0] = window[0][1]; window[0][1] = window[0][2];
		  window[1][0] = window[1][1]; window[1][1] = window[1][2];
		  window[2][0] = window[2][1]; window[2][1] = window[2][2];

		  int prev_row_elem = prev_row[prev_row_index];

		  // grab next column (the rightmost column of the sliding window)
		  window[0][2] = prev_prev_row[prev_prev_row_index];
		  window[1][2] = prev_row_elem;
		  window[2][2] = pixel;

		  //print_window();

		  // shift in new pixel
		  prev_prev_row[prev_prev_row_index] = prev_row_elem;
		  prev_row[prev_row_index] = pixel;

		  // adjust shift register indices
		  prev_row_index++;
		  prev_prev_row_index++;

		  prev_row_index = (prev_row_index==WIDTH) ? 0 : prev_row_index;
		  prev_prev_row_index = (prev_prev_row_index==WIDTH) ? 0 : prev_prev_row_index;

		  // we only want to start calculating the value when
		  // the shift registers are full and the window is valid
		  int check = (y == 2) & (x == 2);
		  x_offset = (check) ? 1: x_offset;
		  y_offset = (check) ? 1: y_offset;
		  start = (!start) ? check : start;

		  int invalid = not_in_bounds(x_offset, y_offset) + !start;

		  x_dir = 0; y_dir = 0;
		  for(yOffset = -1; yOffset <= 1; yOffset++) {
			  for(xOffset = -1; xOffset <= 1; xOffset++) {
				  //printf("%d ", window[1+yOffset][1+xOffset]);
				  x_dir += window[1+yOffset][1+xOffset] * Gx[1+xOffset][1+yOffset];
				  y_dir += window[1+yOffset][1+xOffset] * Gy[1+xOffset][1+yOffset];
			  }
			  //printf("\n");
		  }

		  edge_weight = bound2(x_dir) + bound2(y_dir);
		  edge_weight = 255 - edge_weight;
		  output_image[y_offset][x_offset] = (invalid) ? 0 : edge_weight;
		  //if (!invalid) 
		  if (0) printf("y=%d;x=%d-->SUM=%d+%d=%d--outdata[%d]=%d\n", y_offset, x_offset,
				  x_dir, y_dir,
				  edge_weight, x_offset + y_offset*WIDTH, 
				  (int)output_image[y_offset][x_offset]);

		  x_offset++;

		  y_offset = (x_offset == WIDTH-1) ? (y_offset + 1) : y_offset;
		  x_offset = (x_offset == WIDTH-1) ? -1 : x_offset;
	  }

}



int main()
{
  int x, y;
  
  //sobel(elaine_512_input, output_image);

  //sobel_pipelined(elaine_512_input, output_image);

  //sobel_fast(elaine_512_input, output_image);

  sobel_fast_pipelined(elaine_512_input, output_image);
  printf("Done pipeline\n");

  int result = 0;
  for (y = 0; y < HEIGHT; y++) {
	  for (x = 0; x < WIDTH; x++) {
		  result += output_image[y][x] == elaine_512_golden_output[y][x];
		  if( output_image[y][x] != elaine_512_golden_output[y][x]) {
			  printf("y=%d ; x=%d --> output_image != golden --> %d != %d\n",
					  y, x, output_image[y][x],
					  elaine_512_golden_output[y][x]);
		  }
	  }
  }
    
  if (result == HEIGHT*WIDTH) {
      printf("PASS!\n");
  } else {
      printf("FAIL with %d differences\n", HEIGHT*WIDTH-result);
  }
  
  return result;
}



