#include <stdio.h>
#include "mm.h"

int bitwise_main();
int framing_main();

int main() {
  mm_init();

  int result = (bitwise_main() == 0) + (framing_main() == 0);
  printf("\nResult: %d\n", result);
  if (result == 2) {
      printf("RESULT: PASS\n");
  } else {
      printf("RESULT: FAIL\n");
  }
  return result;
}
