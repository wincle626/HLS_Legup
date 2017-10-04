#include <stdio.h>

// Tests structs with nested arrays and structs.
// Functionality includes loads, stores,
// global and zero initialization.


struct point {
   char x, y;
};

struct str {
   long long a, b;
   short c, d, e;
   char f, g;
   long h;
   struct point p[2];
   long arr[4];
   struct point p2;
   char arr2[4];
};

volatile struct str global[2] = {{0, 1, 2, 3, 4, 5, 6, 7, {{8, 9}, {10, 11}}, {12, 13, 14, 15}, {16, 17}, {18, 19, 20, 21}}, {15, 14, 13, 12, 11, 10, 9, 8, {{7, 6}, {5, 4}}, {3, 2, 1, 0}, {-1, -2}, {-3, -4, -5, -6}}};
volatile struct str zero[5] = {};

// sum up everyithing in a struct point
int pointSum(struct point pt) {
   return pt.x + pt.y;
}

// sum up everything in a struct str
int sum(struct str var) {
   int partsum = 0, i;
   for (i = 0; i < 4; i++) {
      partsum += var.arr[i] + var.arr2[i];
   }
   return var.a + var.b + var.c + var.d + var.e + var.f + var.g + var.h + pointSum(var.p[0]) + pointSum(var.p[1]) + partsum + pointSum(var.p2);
}

int main()
{
   int result = sum(global[0]) + sum(global[1]);
   printf("%d %d %d %d %d\n", global[1].p[1].y, global[0].p2.x, (int)global[1].arr[2], global[0].arr2[3], zero[3].p[1].y);

   // test longer and more complicated GEP's
   result += global[1].p[1].y + global[0].p2.x + global[1].arr[2] + global[0].arr2[3] + zero[3].p[1].y;

   printf("Result: %d\n", result);
   if (result == 372) {
       printf("RESULT: PASS\n");
   } else {
       printf("RESULT: FAIL\n");
   }
   return result;
}

