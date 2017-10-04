#include <stdio.h>

int val(int i, int j)
{
  int a[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  int b[10] = {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};

  return a[i] + b[j];
}

int main()
{
  int result = val(1, 1);
  printf("Result: %d\nExpected: %d\n", result, 9);
  return result;
}
