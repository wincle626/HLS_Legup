#define DIM_X 60
#define DIM_Y 30
#define NUM_ACCEL 4
#define OMP_ACCEL 3
#define OPS_PER_ACCEL DIM_Y/NUM_ACCEL

#include <stdio.h>
#include "los.h"
#include <pthread.h>

struct thread_data{
   int  startidx;
   int  maxidx;
};

//x0 and x0 are the coordinates of the pixel under consideration, which will move towards the person
//x1 and y1 are the coordinates of the location of the person
void *line (void *threadarg) {
	int sx, sy, err, dx, dy, e2, x_pixel, y_pixel, tid, i;
	int sight;
	int result=0;
	int x0, y0;
	struct thread_data* arg = (struct thread_data*) threadarg;
	int y_start = arg->startidx;
	int y_end = arg->maxidx;

	const int x1=30;
	const int y1=15;

	//int temp[OMP_ACCEL]={0};
	int visible=0;
	int temp[OMP_ACCEL];
	for (y_pixel=y_start; y_pixel<y_end; y_pixel++) {
	    //#pragma omp parallel for num_threads(OMP_ACCEL) private(x_pixel, tid)
	    #pragma omp parallel for num_threads(OMP_ACCEL) private(x_pixel, tid, sight, x0, y0, dx, dy, sx, sy, err, e2)
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
				output[y_pixel][x_pixel] = 1;
                temp[tid]++;
			}
		}
	}

    for (i=0; i<OMP_ACCEL; i++) {
        visible += temp[i];
    }
    pthread_exit((void*)visible);
}

int main() {
    legup_start_counter(0);
	int i;
	int sum=0;
	int result[NUM_ACCEL];
	pthread_t threads[NUM_ACCEL];
	struct thread_data data[NUM_ACCEL];

	//initialize structs to pass into accels
	for (i=0; i<NUM_ACCEL; i++) {
		data[i].startidx = i*OPS_PER_ACCEL;
		data[i].maxidx = (i+1)*OPS_PER_ACCEL;
	}

	for (i=0; i<NUM_ACCEL; i++) {
		pthread_create(&threads[i], NULL, line, (void *)&data[i]);
	}

	//join the threads
	for (i=0; i<NUM_ACCEL; i++) {
		pthread_join(threads[i], (void**)&result[i]);
	}

	for (i=0; i<NUM_ACCEL; i++) {
		sum += result[i];
	}

    int perf_counter = legup_stop_counter(0);
    printf("perf_counter = %d\n", perf_counter);
    //check final result
	printf ("Sum: %d\n", sum);
	if (sum == 1202) {
		printf("RESULT: PASS\n");
	} else {
		printf("RESULT: FAIL\n");
	}

	return 0;
}
