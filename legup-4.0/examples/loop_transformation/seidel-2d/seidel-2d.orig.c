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
	for (t = 0; t <= TSTEPS - 1; t++) {
		for (i = 1; i<= N - 2; i++) {
l1:			for (j = 1; j <= N - 2; j++) {
				A[i][j] = (A[i-1][j-1] + A[i-1][j] + A[i-1][j+1]
						+ A[i][j-1] + A[i][j] + A[i][j+1]
						+ A[i+1][j-1] + A[i+1][j] + A[i+1][j+1])/9.0;
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
