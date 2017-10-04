/**
 * This file is based on the gemver benchmark in PolyBench/C 3.2 test suite.
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

/* Include benchmark-specific header. */
/* Default data type is double, default size is 4000. */
#include "gemver.h"

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

	/* Variable declaration/allocation. */
	DATA_TYPE alpha = 0;
	DATA_TYPE beta = 0;
	DATA_TYPE A[N][N] = {0};
	DATA_TYPE u1[N] = {0};
	DATA_TYPE v1[N] = {0};
	DATA_TYPE u2[N] = {0};
	DATA_TYPE v2[N] = {0};
	DATA_TYPE w[N] = {0};
	DATA_TYPE x[N] = {0};
	DATA_TYPE y[N] = {0};
	DATA_TYPE z[N] = {0};

	int i, j;

	/* Initialize array(s). */
	alpha = 43532;
	beta = 12313;

	for (i = 0; i < n; i++) {
		u1[i] = i;
		u2[i] = (i+1)/n/2.0;
		v1[i] = (i+1)/n/4.0;
		v2[i] = (i+1)/n/6.0;
		y[i] = (i+1)/n/8.0;
		z[i] = (i+1)/n/9.0;
		x[i] = 0.0;
		w[i] = 0.0;
		for (j = 0; j < n; j++)
			A[i][j] = ((DATA_TYPE) i*j) / n;
	}

	/************** 
	 * Run kernel. 
	 **************/
	printf("### started ###\n");
#pragma scop
	for (i = 0; i < N; i++)
l1:		for (j = 0; j < N; j++)
			A[i][j] = A[i][j] + u1[i] * v1[j] + u2[i] * v2[j];

	for (i = 0; i < N; i++)
l2:		for (j = 0; j < N; j++)
			x[i] = x[i] + beta * A[j][i] * y[j];

l3:	for (i = 0; i < N; i++)
		x[i] = x[i] + z[i];

	for (i = 0; i < N; i++)
l4:		for (j = 0; j < N; j++)
			w[i] = w[i] +  alpha * A[i][j] * x[j];
#pragma endscop
	printf("### finished ###\n");

	DATA_TYPE scram = 0;
	for (i=0; i<N; i++)
		for (j=0; j<N; j++)
			scram = scramble ( A[i][j], scram );
	for (i=0; i<N; i++)
		scram = scramble ( x[i], scram );
	for (i=0; i<N; i++)
		scram = scramble ( w[i], scram );

	int scram_val = (int) scram;
	printf("Scrambled Data = %d \n", scram_val);

	return scram_val;
}
