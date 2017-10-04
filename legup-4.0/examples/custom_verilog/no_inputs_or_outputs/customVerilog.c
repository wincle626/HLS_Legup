// Custom Verilog
// Author: Mathew Hall
// Date: May 6, 2014

#include <stdio.h>

int __attribute__ ((noinline)) customAdd(int i, int j) {
  return i+j;
}

int main(void) {
    int i = 1, j = 5;
    int k = customAdd(i,j);
    printf("Result: %d\n", k);
    if (k == 6)
      printf("RESULT: PASS\n");
    else
      printf("RESULT: FAIL\n");
    return customAdd(i,j);
}


