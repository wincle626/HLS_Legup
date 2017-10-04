#include <stdio.h>
#include "mm.h"

#define LEN 24

int main() {
  mm_init();

  char *ptr = (char *)mm_malloc(LEN);
  int i, sum = 0;

  for (i = 0; i < LEN; i++)
    ptr[i] = i;

  char *ptr2 = (char *)mm_malloc(LEN * 2);
  for (i = 0; i < LEN * 2; i++)
    ptr2[i] = 5;

  ptr = (char *)mm_realloc(ptr, LEN * 2);
  for (i = LEN; i < LEN * 2; i++)
    ptr[i] = i;

  for (i = 0; i < LEN * 2; i++) {
    sum += ptr[i];
    sum += ptr2[i];
  }

  printf("Result: %d\n", sum);
  if (sum == 1368) {
      printf("RESULT: PASS\n");
  } else {
      printf("RESULT: FAIL\n");
  }
  return sum;
}
