
#include <stdio.h>
#include <pthread.h>
#include "legup_mem.h"
#include <stdlib.h>

#define SIZE 16
#define NUM_THREADS 8

struct args {
    char *a, *b;
}; 

void vector_add(char * a, char b)
{
  *a = *a + b;
}

void* vector_add_thread (void * args)
{
  char *a = ((struct args *)args)->a;
  char *b = ((struct args *)args)->b;
  vector_add(a, *b);

  return NULL;
}

void vector_add_loop(char *a, char *b)
{
  pthread_t pool[NUM_THREADS];
  struct args arg_array[NUM_THREADS];

  int i;
  for (i = 0; i < NUM_THREADS; i++) {
    arg_array[i] = (struct args){a + i, b + i};
    pthread_create(pool + i, NULL, vector_add_thread, (void *)(arg_array + i));
  }
  for (i = 0; i < NUM_THREADS; i++) {
    pthread_join(pool[i], NULL);
  }
}

#ifndef LIB_TARGET
int main()
{
  char b[SIZE], golden[SIZE];
  char * a = malloc(SIZE * sizeof(char));
  char * a_shared = malloc_shared(SIZE * sizeof(char), a, LEGUP_RAM_LOCATION_DDR2);
  int i, result = 0;
  for (i = 0; i < SIZE; i++) {
    a[i] = (char)(i * 5);
    b[i] = (char)(15 - i);
    golden[i] = a[i] + b[i];
  }
  memcpy_to_shared(a_shared, a, SIZE * sizeof(char));

  for (i = 0; i < SIZE / NUM_THREADS; i++) {
    vector_add_loop(a_shared + i * NUM_THREADS, b + i * NUM_THREADS);
  }

  memcpy_from_shared(a, a_shared, SIZE * sizeof(char));

  for (i = 0; i < SIZE; i++) {
    printf("Expected %d, calculated %d\n", golden[i], a[i]);
    result += (a[i] == golden[i]);
  }

  if (result == SIZE) {
    printf("PASS\n");
  } else {
    printf("FAILED %d out of %d\n", SIZE-result, SIZE);
  }
  free (a);
  free_shared (a_shared);

  return result;
}
#endif // LIB_TARGET

