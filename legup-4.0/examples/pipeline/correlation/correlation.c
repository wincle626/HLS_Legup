#include <math.h>
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
	volatile DATA_TYPE data  [M][N];
	volatile DATA_TYPE symmat[M][M];
	volatile DATA_TYPE mean  [M];
	volatile DATA_TYPE stddev[M];

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
#ifdef ceild
# undef ceild
#endif
#ifdef floord
# undef floord
#endif
#ifdef max
# undef max
#endif
#ifdef min
# undef min
#endif
#define ceild(x,y) (((x) > 0)? (1 + ((x) - 1)/(y)): ((x) / (y)))
#define floord(x,y) (((x) > 0)? ((x)/(y)): 1 + (((x) -1)/ (y)))
#define max(x,y)    ((x) > (y)? (x) : (y))
#define min(x,y)    ((x) < (y)? (x) : (y))
  register int lbv, ubv, lb, ub, lb1, ub1, lb2, ub2;
  register int c1, c2, c3;
#pragma scop
symmat[_PB_M-1][_PB_M-1] = 1.0;
for (c1 = 1; c1 <= ((2 * _PB_M) + -3); c1++) {
loop_1:  for (c2 = ceild((c1 + 1), 2); c2 <= min(c1, (_PB_M + -1)); c2++) {
    symmat[(c1 + (-1 * c2))][c2] = 0.0;
  }
}
loop_2:	for (c1 = 0; c1 <= (_PB_M + -2); c1++) {
  symmat[c1][c1] = 1.0;
  stddev[c1] = 0.0;
  mean[c1] = 0.0;
}
if ((_PB_M >= 1)) {
  stddev[(_PB_M + -1)] = 0.0;
  mean[(_PB_M + -1)] = 0.0;
}
if (((_PB_M >= 1) && (_PB_N >= 1))) {
  for (c1 = 0; c1 <= ((_PB_M + _PB_N) + -2); c1++) {
loop_3:	    for (c2 = max(0, ((c1 + (-1 * _PB_M)) + 1)); c2 <= min(c1, (_PB_N + -1)); c2++) {
      mean[(c1 + (-1 * c2))] += data[c2][(c1 + (-1 * c2))];
    }
  }
}
loop_4:	for (c1 = 0; c1 <= (_PB_M + -1); c1++) {
  mean[c1] /= float_n;
}
if (((_PB_M >= 1) && (_PB_N >= 1))) {
  for (c1 = 0; c1 <= ((_PB_M + _PB_N) + -2); c1++) {
loop_5:	    for (c2 = max(0, ((c1 + (-1 * _PB_M)) + 1)); c2 <= min(c1, (_PB_N + -1)); c2++) {
      stddev[(c1 + (-1 * c2))] += (data[c2][(c1 + (-1 * c2))] - mean[(c1 + (-1 * c2))]) * (data[c2][(c1 + (-1 * c2))] - mean[(c1 + (-1 * c2))]);
      data[c2][(c1 + (-1 * c2))] -= mean[(c1 + (-1 * c2))];
    }
  }
}
loop_6:	for (c1 = 0; c1 <= (_PB_M + -1); c1++) {
  stddev[c1] /= float_n;
  stddev[c1] = sqrt_of_array_cell(stddev, c1);
  stddev[c1] = stddev[c1] <= eps ? 1.0 : stddev[c1];
}
if (((_PB_M >= 1) && (_PB_N >= 1))) {
  for (c1 = 0; c1 <= ((_PB_M + _PB_N) + -2); c1++) {
loop_7:	    for (c2 = max(0, ((c1 + (-1 * _PB_N)) + 1)); c2 <= min(c1, (_PB_M + -1)); c2++) {
      data[(c1 + (-1 * c2))][c2] /= sqrt(float_n) * stddev[c2];
    }
  }
}
if ((_PB_N >= 1)) {
  for (c1 = 1; c1 <= ((2 * _PB_M) + -3); c1++) {
    for (c2 = ceild((c1 + 1), 2); c2 <= min(c1, (_PB_M + -1)); c2++) {
loop_8:	      for (c3 = 0; c3 <= (_PB_N + -1); c3++) {
        symmat[(c1 + (-1 * c2))][c2] += (data[c3][(c1 + (-1 * c2))] * data[c3][c2]);
      }
    }
  }
}
for (c1 = 1; c1 <= ((2 * _PB_M) + -3); c1++) {
loop_9:	  for (c2 = ceild((c1 + 1), 2); c2 <= min(c1, (_PB_M + -1)); c2++) {
    symmat[c2][(c1 + (-1 * c2))] = symmat[(c1 + (-1 * c2))][c2];
  }
}
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
