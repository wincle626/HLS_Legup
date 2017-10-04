#include <stdio.h>

#include "legup_mem.h"

#define SIZE 16

void vector_add(char *a, char *b, char *c, int len)
{
  int i;
  for (i = 0; i < len; i++) {
    c[i] = a[i] + b[i];
  }
}

int main()
{
  char a[SIZE], b[SIZE], c[SIZE], golden[SIZE];
  int i, result = 0;
  for (i = 0; i < SIZE; i++) {
    a[i] = (char)(i * 5);
    b[i] = (char)(15 - i);
    c[i] = 0;
    golden[i] = a[i] + b[i];
  }

  // pre-accelerator copy and allocate
  char *SHARED_MEM_a = malloc_shared(SIZE*sizeof(char), a);
  memcpy_to_shared(SHARED_MEM_a, a, SIZE*sizeof(char));
  char *SHARED_MEM_b = malloc_shared(SIZE*sizeof(char), b);
  memcpy_to_shared(SHARED_MEM_b, b, SIZE*sizeof(char));
  char *SHARED_MEM_c = malloc_shared(SIZE*sizeof(char), c);

  vector_add(SHARED_MEM_a, SHARED_MEM_b, SHARED_MEM_c, SIZE);

  // post-accelerator copy and free
  memcpy_from_shared(c, SHARED_MEM_c, SIZE*sizeof(char));
  free_shared(SHARED_MEM_a);
  free_shared(SHARED_MEM_b);
  free_shared(SHARED_MEM_c);

  for (i = 0; i < SIZE; i++) {
    printf("Expected %d, calculated %d\n", golden[i], c[i]);
    result += (c[i] == golden[i]);
  }

  if (result == SIZE) {
    printf("PASS\n");
  } else {
    printf("FAILED %d out of %d\n", SIZE-result, SIZE);
  }

  return result;
}
