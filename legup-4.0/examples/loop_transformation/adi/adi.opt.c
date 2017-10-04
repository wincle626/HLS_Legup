/**
 * This file is based on the adi benchmark in PolyBench/C 3.2 test suite.
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

/* Include benchmark-specific header. */
/* Default data type is double, default size is 10x1024x1024. */
#include "adi.h"

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
	DATA_TYPE X [N][N] = {0};
	DATA_TYPE A [N][N] = {0};
	DATA_TYPE B [N][N] = {0};

	int i, j;

	/* Initialize array(s). */
	for (i = 0; i < n; i++)
		for (j = 0; j < n; j++) {
			X[i][j] = ((DATA_TYPE) i*(j+1) + 1) / n;
			A[i][j] = ((DATA_TYPE) i*(j+2) + 2) / n;
			B[i][j] = ((DATA_TYPE) i*(j+3) + 3);
		}


	/************** 
	 * Run kernel. 
	 **************/
	int t, i1, i2;

	printf("### started ###\n");
#pragma scop
	for (t = 0; t < TSTEPS; t++) {
		for (i2 = 1; i2 < N; i2++) {
l1:			for (i1 = 0; i1 < N; i1++) {
				X[i1][i2] = X[i1][i2] - X[i1][i2-1] * A[i1][i2] / B[i1][i2-1];
				B[i1][i2] = B[i1][i2] - A[i1][i2] * A[i1][i2] / B[i1][i2-1];
			}
		}

l2:		for (i1 = 0; i1 < N; i1++) {
			X[i1][N-1] = X[i1][N-1] / B[i1][N-1];
		}

		for (i2 = 0; i2 < N-2; i2++) {
l3:			for (i1 = 0; i1 < N; i1++) {
				X[i1][N-i2-2] = (X[i1][N-2-i2] - X[i1][N-2-i2-1] * A[i1][N-i2-3]) / B[i1][N-3-i2];
			}
		}

		for (i1 = 1; i1 < N; i1++) {
l4:			for (i2 = 0; i2 < N; i2++) {
				X[i1][i2] = X[i1][i2] - X[i1-1][i2] * A[i1][i2] / B[i1-1][i2];
				B[i1][i2] = B[i1][i2] - A[i1][i2] * A[i1][i2] / B[i1-1][i2];
			}
		}

l5:		for (i2 = 0; i2 < N; i2++) {
			X[N-1][i2] = X[N-1][i2] / B[N-1][i2];
		}

		for (i1 = 0; i1 < N-2; i1++) {
l6:			for (i2 = 0; i2 < N; i2++) {
				X[N-2-i1][i2] = (X[N-2-i1][i2] - X[N-i1-3][i2] * A[N-3-i1][i2]) / B[N-2-i1][i2];
			}
		}
	}
#pragma endscop
	printf("### finished ###\n");

	DATA_TYPE scram = 0;
	for (i=0; i<N; i++) {
		for (j=0; j<N; j++) {
			scram = scramble ( X[i][j], scram );
			scram = scramble ( A[i][j], scram );
			scram = scramble ( B[i][j], scram );
		}
	}
	int scram_val = (int) scram;
	printf("Scrambled Data = %d\n", scram_val);
	return scram_val;
}
