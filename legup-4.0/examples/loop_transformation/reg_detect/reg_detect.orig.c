/**
 * This file is based on the reg_detect benchmark in PolyBench/C 3.2 test suite.
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

/* Include benchmark-specific header. */
/* Default data type is double, default size is 4000. */
#include "reg_detect.h"

DATA_TYPE scramble (DATA_TYPE num, DATA_TYPE scram) {
	if (num<scram)
		return scram-num;
	if (num>scram)
		return num-scram;
	return scram;
}

int main(int argc, char** argv)
{
	DATA_TYPE sum_tang [MAXGRID][MAXGRID] = {0};
	DATA_TYPE mean [MAXGRID][MAXGRID] = {0};
	DATA_TYPE path [MAXGRID][MAXGRID] = {0};
	DATA_TYPE diff [MAXGRID][MAXGRID][LENGTH] = {0};
	DATA_TYPE sum_diff [MAXGRID][MAXGRID][LENGTH] = {0};

	/* Retrieve problem size. */
	int niter = NITER;
	int maxgrid = MAXGRID;
	int length = LENGTH;

	int t, i, j, cnt;

	// initialize data array
	for (i = 0; i < maxgrid; i++) {
		for (j = 0; j < maxgrid; j++) {
			sum_tang[i][j] = (DATA_TYPE)((i+1)*(j+1));
			mean[i][j] = ((DATA_TYPE) i-j);
			path[i][j] = ((DATA_TYPE) i*(j-1));
			for (cnt = 0; cnt < length; cnt++) {
				diff[i][j][cnt] = 0;
				sum_diff[i][j][cnt] = 0;
			}
		}
	}

	/************** 
	 * Run kernel. 
	 **************/
	printf("### started ###\n");
#pragma scop
 	for (t = 0; t < NITER; t++) {

		for (j = 0; j <= MAXGRID - 1; j++) {
			for (i = j; i <= MAXGRID - 1; i++) {
l1:				for (cnt = 0; cnt <= LENGTH - 1; cnt++) {
					diff[j][i][cnt] = sum_tang[j][i];
				}
			}
		}

		for (j = 0; j <= MAXGRID - 1; j++) {
			for (i = j; i <= MAXGRID - 1; i++) {
				sum_diff[j][i][0] = diff[j][i][0];
l2:				for (cnt = 1; cnt <= LENGTH - 1; cnt++) {
					sum_diff[j][i][cnt] = sum_diff[j][i][cnt - 1] + diff[j][i][cnt];
				}
				mean[j][i] = sum_diff[j][i][LENGTH - 1];
			}
		}

l3:		for (i = 0; i <= MAXGRID - 1; i++) {
			path[0][i] = mean[0][i];
		}

		for (j = 1; j <= MAXGRID - 1; j++) {
l4:			for (i = j; i <= MAXGRID - 1; i++) {
				path[j][i] = path[j - 1][i - 1] + mean[j][i];
			}
		}
	}
#pragma endscop
	printf("### finished ###\n");

	DATA_TYPE scram = 0;

	for (i=0; i<MAXGRID; i++) {
		for (j=0; j<MAXGRID; j++) {
			scram = scramble ( sum_tang [i][j], scram );
			scram = scramble ( mean[i][j], scram );
			scram = scramble ( path[i][j], scram );
			for (cnt = 0; cnt<LENGTH; cnt++) {
				scram = scramble ( diff[i][j][cnt], scram );
				scram = scramble ( sum_diff[i][j][cnt], scram );
			}
		}
	}
	printf("Scrambled Data = %d\n", scram);
	return scram;
}
