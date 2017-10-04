#define DIM_X 60
#define DIM_Y 60
#define NUM_ACCEL 1
#define OMP_ACCEL 4
#define OPS_PER_ACCEL DIM_Y/NUM_ACCEL

#include <stdio.h>
#include "los.h"

//#define LEGUP 1

//x0 and x0 are the coordinates of the pixel under consideration, which will move towards the person
//x1 and y1 are the coordinates of the location of the person
int line () {
	int sx, sy, err, dx, dy, e2, x_pixel, y_pixel, tid, i;
	int sight=0;
	int result=0;
	int x0=0, y0=0;

	const int x1=DIM_X/2;
	const int y1=DIM_Y/2;

	int temp[OMP_ACCEL]={0};
	int visible=0;
	//int temp[OMP_ACCEL];
	#pragma omp parallel for num_threads(OMP_ACCEL) private(y_pixel, x_pixel, tid, sight, x0, y0, dx, dy, sx, sy, err, e2)
	for (y_pixel=0; y_pixel<DIM_Y/NUM_ACCEL; y_pixel++) {
	   // #pragma omp parallel for num_threads(OMP_ACCEL) private(x_pixel, tid, sight, x0, y0, dx, dy, sx, sy, err, e2) reduction(+ :visible)
		for (x_pixel=0; x_pixel<DIM_X; x_pixel++) {
            tid = omp_get_thread_num();
			sight=1;
			x0 = x_pixel;
			y0 = y_pixel;
			if (x0 < x1) {
				sx=1;
				dx=x1-x0;
			}
			else {
				sx=-1;
				dx=x0-x1;
			}

			if (y0 < y1) {
				sy=1;
				dy=y1-y0;
			}
			else {
				sy=-1;
				dy=y0-y1;
			}
	
			err=dx-dy;

			while (1) {
				if ((x0==x1) && (y0==y1)) {
					break;
				}
				//if there's an obstacle
				if (obstacles[y0][x0] == 1) {
					sight=0;
					break;
				}
				e2=2*err;
				if (e2 > -dy) {
					err = err - dy;
					x0 = x0+sx;
				}
		
				if (e2 < dx) {
					err = err + dx;
					y0 = y0 + sy;
				}
			}	
	
			//if an obstacle was not found on the way
			if (sight == 1) {
				//store 1 on the output
				//output[y_pixel+offset][x_pixel] = 1;
				output[y_pixel][x_pixel] = 1;
                //visible++;
                temp[tid]++;
			}
		}
	}

    for (i=0; i<OMP_ACCEL; i++) {
        visible += temp[i];
    }
    return visible;
}

int main() {
    #ifdef LEGUP
    legup_start_counter(0);
    #endif
	int i, j;
	int sum=0;
	int result[NUM_ACCEL];

    sum = line();
/*
    // checking output
  	for (i=0; i<DIM_Y; i++) {
		for (j=0; j<DIM_X; j++) {
            printf("%d", output[i][j]);
        }
        printf("\n");
    }
*/
    #ifdef LEGUP
    int perf_counter = legup_stop_counter(0);
    printf("perf_counter = %d\n", perf_counter);
    #endif
    //check final result
	printf ("Sum: %d\n", sum);
	if (sum == 2193) {
		printf("RESULT: PASS\n");
	} else {
		printf("RESULT: FAIL\n");
	}

	return 0;
}
