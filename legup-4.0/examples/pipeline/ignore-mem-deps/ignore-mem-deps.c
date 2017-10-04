/**
 * correlation.c: This file is part of the PolyBench/C 3.2 test suite.
 *
 *
 * Contact: Louis-Noel Pouchet <pouchet@cse.ohio-state.edu>
 * Web address: http://polybench.sourceforge.net
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#define DATA_TYPE int
#define M 20
#define N 20
#define _PB_M M
#define _PB_N N

DATA_TYPE scramble (DATA_TYPE num, DATA_TYPE scram) {
	if (num<scram)
		return scram-num;
	if (num>scram)
		return num-scram;
	return scram;
}

int main()
{
	 DATA_TYPE data  [M][N];
	 DATA_TYPE symmat[M][M];
	 DATA_TYPE mean  [M];
	 DATA_TYPE stddev[M];

	/* Retrieve problem size. */
	int n = N;
	int m = M;

	int i, j, j1, j2;

	// initialize data array
	for (i = 0; i < m; i++)
		for (j = 0; j < n; j++)
			data[i][j] = ((DATA_TYPE) i*(j+1));

	DATA_TYPE eps = 1;
  	DATA_TYPE float_n = 3;

#define sqrt_of_array_cell(x,j) sqrt(x[j])

	printf("started.\n");
#pragma scop
	/* Determine mean of column vectors of input data matrix */
	for (j = 0; j < _PB_M; j++)
	{
		mean[j] = 0.0;
loop_1:	for (i = 0; i < _PB_N; i++) {
			mean[j] += data[i][j];
		}
		mean[j] /= float_n;
	}

	/* Determine standard deviations of column vectors of data matrix. */
	for (j = 0; j < _PB_M; j++)
	{
		stddev[j] = 0.0;
loop_2:	for (i = 0; i < _PB_N; i++) {
			stddev[j] += (data[i][j] - mean[j]) * (data[i][j] - mean[j]);
		}
		stddev[j] /= float_n;
		stddev[j] = sqrt_of_array_cell(stddev, j);
		/* The following in an inelegant but usual way to handle
		   near-zero std. dev. values, which below would cause a zero-
		   divide. */
		stddev[j] = stddev[j] <= eps ? 1.0 : stddev[j];
	}

	/* Center and reduce the column vectors. */
	for (i = 0; i < _PB_N; i++)
loop_3:	for (j = 0; j < _PB_M; j++)
		{
			data[i][j] -= mean[j];
			data[i][j] /= sqrt(float_n) * stddev[j];
		}

	/* Calculate the m * m correlation matrix. */
	for (j1 = 0; j1 < _PB_M-1; j1++)
	{
		symmat[j1][j1] = 1.0;
		for (j2 = j1+1; j2 < _PB_M; j2++)
		{
			symmat[j1][j2] = 0.0;
loop_4:		for (i = 0; i < _PB_N; i++) {
				symmat[j1][j2] += (data[i][j1] * data[i][j2]);
			}
			symmat[j2][j1] = symmat[j1][j2];
		}
	}
	symmat[_PB_M-1][_PB_M-1] = 1.0;
#pragma endscop
	printf("finished.\n");

	DATA_TYPE scram = 0;

	for (i=0; i<M; i++)
		for (j=0; j<N; j++)
			scram = scramble ( data[i][j], scram );

	for (i=0; i<M; i++)
		for (j=0; j<M; j++)
			scram = scramble ( symmat[i][j], scram );
	
	for (i=0; i<M; i++)
		scram = scramble ( mean[i], scram );

	for (i=0; i<M; i++)
		scram = scramble ( stddev[i], scram );

	printf("Scrambled Data = %d\n", scram);
	return scram;
}
