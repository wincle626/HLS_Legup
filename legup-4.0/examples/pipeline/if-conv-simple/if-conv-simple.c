#include <stdio.h>

// Simple accumulation
int main() {
#define N 10
    volatile int a[N] = {-1,2,-3,4,-5,6,-7,8,-9,10};
    int sum = 0;
    int i;
loop: for (i = 0; i < N; i++) {
      // this conditional will result in control
      // flow 
      if (a[i] > 0) {
            sum += a[i] + i;
      } else {
            sum += a[i] * i;
      }
    printf("sum %d = %d\n", i, sum);
}
    printf("sum = %d\n", sum);

    return sum;
}
