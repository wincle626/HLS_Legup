/**
 * This file is based on the seidel-2d benchmark in PolyBench/C 3.2 test suite.
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

/* Include benchmark-specific header. */
/* Default data type is double, default size is 20x1000. */
#include "seidel-2d.h"

DATA_TYPE scramble (DATA_TYPE num, DATA_TYPE scram) {
	if (num<scram)
		return scram-num;
	if (num>scram)
		return num-scram;
	return scram;
}

int main(int argc, char** argv) {
	/* Retrieve problem size. */
	int n = N;
	int tsteps = TSTEPS;

	/* Variable declaration/allocation. */
	DATA_TYPE A[N][N] = {0};

	int t, i, j;

	/* Initialize array(s). */
	for (i = 0; i < n; i++)
		for (j = 0; j < n; j++)
			A[i][j] = ((DATA_TYPE) i*(j+2) + 2) / n;


	/************** 
	 * Run kernel. 
	 **************/
	printf("### started ###\n");
#pragma scop
#define ceild(x,y) (((x) > 0)? (1 + ((x) - 1)/(y)): ((x) / (y)))
#define floord(x,y) (((x) > 0)? ((x)/(y)): 1 + (((x) -1)/ (y)))
#define max(x,y)    ((x) > (y)? (x) : (y))
#define min(x,y)    ((x) < (y)? (x) : (y))
#pragma scop
	for (t = 0; t <= TSTEPS - 1; t++) {
		for (i = 3; i <= ((3 * N) + -6); i++) {
l1:			for (j = max(ceild((i + 1), 2), ((i + (-1 * N)) + 2)); j <= min(floord(((i + N) + -2), 2), (i + -1)); j++) {
				A[(i + (-1 * j))][((-1 * i) + (2 * j))] = (
						A[(i + (-1 * j))-1][((-1 * i) + (2 * j))-1] + 
						A[(i + (-1 * j))-1][((-1 * i) + (2 * j))  ] + 
						A[(i + (-1 * j))-1][((-1 * i) + (2 * j))+1] + 
						A[(i + (-1 * j))  ][((-1 * i) + (2 * j))-1] + 
						A[(i + (-1 * j))  ][((-1 * i) + (2 * j))  ] + 
						A[(i + (-1 * j))  ][((-1 * i) + (2 * j))+1] + 
						A[(i + (-1 * j))+1][((-1 * i) + (2 * j))-1] + 
						A[(i + (-1 * j))+1][((-1 * i) + (2 * j))  ] + 
						A[(i + (-1 * j))+1][((-1 * i) + (2 * j))+1]
						)/9.0;
			}
		}
	}
#pragma endscop
	printf("### finished ###\n");

	DATA_TYPE scram = 0;
	for (i=0; i<N; i++) {
		for (j=0; j<N; j++) {
			scram = scramble ( A[i][j], scram );
		}
	}
	int scram_val = (int) scram;
	printf("Scrambled Data = %d\n", scram_val);
	return scram_val;

	return 0;
}
