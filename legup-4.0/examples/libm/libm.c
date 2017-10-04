#include <math.h>
#include <stdio.h>

// http://xkcd.com/217/

#define GOLDEN 19999099

int main()
{
  double pi = 4 * atan(1);
  double test = exp(pi) - pi;
  int result = (int)(test * 1000000);

  printf("Result: %d\n", result);
  if (result == GOLDEN) {
    printf("PASS\n");
  } else {
    printf("FAIL\n");
    printf("Expected: %d\n", GOLDEN);
  }

  return result;
}
