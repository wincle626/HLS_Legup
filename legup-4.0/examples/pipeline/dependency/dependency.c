#include <math.h>
#include <stdio.h>

// Simple loop with an array
int main() {
	int i;
	int j;

#define N 100
#define d_i 1
#define d_j 1

	volatile int a[N][N] = {0};
	volatile int b[N][N] = {0};
	volatile int rand = 199;

	for ( j = 0; j < N; j++) {
		for (i = 0; i < N; i++) {
			a[i][j] = i;
			b[i][j] = j;
		}
	}

	int offset = (d_i > d_j) ? d_i: d_j;

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
  register int c0, c1;
#pragma scop
if (offset <= (N + -1)) {
  for (c0 = (2 * offset); c0 <= ((2 * N) + -2); c0++) {
loop:    for (c1 = max(offset, ((c0 + (-1 * N)) + 1)); c1 <= min((N + -1), (c0 + (-1 * offset))); c1++) {
      b[c1][(c0 + (-1 * c1))] += a[c1-d_i][(c0 + (-1 * c1))] + a[c1][(c0 + (-1 * c1))-d_j];
      a[c1][(c0 + (-1 * c1))] = b[c1][(c0 + (-1 * c1))] / rand;
    }
  }
}
#pragma endscop
	printf("finished.\n");

	int sum = 0;
	for ( i = 0; i < N; i++) {
		for ( j = 0; j < N; j++) {
			sum += a[i][j];
		}
	}

	printf("sum=%d\n", sum);
	return sum;
}

