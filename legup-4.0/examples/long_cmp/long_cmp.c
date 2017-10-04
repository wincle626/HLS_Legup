#include <stdio.h>

int main() {
  volatile long long a = 0x02fffff000000001LL;
  volatile long long b = 0x01000000ffffffffLL;

  int x = (a > b);

  printf("Result: %d\n", x);
  if (x == 1) {
      printf("RESULT: PASS\n");
  } else {
      printf("RESULT: FAIL\n");
  }
  return x;

}
