#include <stdio.h>
#include <pthread.h>

#define SIZE 16
#define NUM_THREADS 8

struct args {
    char *a, *b, *sum;
}; 

char vector_add(char a, char b)
{
  return a + b;
}

void* vector_add_thread (void * args)
{
  char *a = ((struct args *)args)->a;
  char *b = ((struct args *)args)->b;
  char *sum = ((struct args *)args)->sum;
  *sum = vector_add(*a, *b);

  return NULL;
}

void vector_add_loop(char *a, char *b, char *sum)
{
  pthread_t pool[NUM_THREADS];
  struct args arg_array[NUM_THREADS];

  int i;
  for (i = 0; i < NUM_THREADS; i++) {
    arg_array[i] = (struct args){a + i, b + i, sum + i};
    pthread_create(pool + i, NULL, vector_add_thread, (void *)(arg_array + i));
  }
  for (i = 0; i < NUM_THREADS; i++) {
    pthread_join(pool[i], NULL);
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

  for (i = 0; i < SIZE / NUM_THREADS; i++) {
    vector_add_loop(a + i * NUM_THREADS, b + i * NUM_THREADS, c + i * NUM_THREADS);
  }

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
