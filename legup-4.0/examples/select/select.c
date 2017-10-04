#include <stdio.h>

int main() {
   volatile int a = 1, c = 0;

   // test true case
   int b = (a) ? 8 : 3;
   // test false case
   int d = (c) ? 5 : 7;

   printf("%d %d\n", b, d);

   int result = b + d;
   printf("Result: %d\n", result);
   if (result == 15) {
       printf("RESULT: PASS\n");
   } else {
       printf("RESULT: FAIL\n");
   }
   return result;
   
}
