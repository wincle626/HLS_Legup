#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_ITER 500
#define RAND_MAX 1000
int main (int argc, char *argv[])
{
  /* Simple "srand()" seed: just use "time()" */
  unsigned int iseed = (unsigned int)time(NULL);
  srand (iseed);

  /* Now generate 5 pseudo-random numbers */
  int i, num;
  for (i=0; i<MAX_ITER; i++)
  {
	num = 2 + (int)( 10.0 * rand() / ( RAND_MAX + 1.0 ) );	

    printf ("%u, \n", num);
  }
  return 0;
}
