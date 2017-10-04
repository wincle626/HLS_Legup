// This tests the conversion of llvm.memset and llvm.memcpy
// intrinsics into aligned calls of our own functions,
// defined in intrinsics/intrinsics.c

#include <stdio.h>

#define LEN 12

// produce llvm.memset's
void empty_byte(char * arr) {
   int i;
   for (i = 0; i < LEN; i++)
      *arr++ = 0;
}

void empty_short(short * arr) {
   int i;
   for (i = 0; i < LEN; i++)
      *arr++ = 0;
}

void empty_word(int * arr) {
   int i;
   for (i = 0; i < LEN; i++)
      *arr++ = 0;
}

void empty_long(long long * arr) {
   int i;
   for (i = 0; i < LEN; i++)
      *arr++ = 0;
}

int arr_sum (char * carr, short * sarr, int * arr, long long * larr) {
   int i, sum = 0;
   for (i = 0; i < LEN; i++)
      sum += (int) carr[i] + (int) sarr[i] + arr[i] + (int)larr[i];
   return sum;
}

int main() {
    // produce llvm.memcpy's
    char carray[LEN] = {1, 3, 5, 3, 2, 5, 3, 5, 3, 5, -2, 100};
    short int sarray[LEN] = {-1, -3, -5, -3, -2, -5, -3, -5, -3, -5, 2, 100};
    int array[LEN] = {-1, -3, -5, -3, -2, -5, -3, -5, -3, -5, 2, 100};
    long long larray[LEN] = {1, 3, 5, 3, 2, 5, 3, 5, 3, 5, -2, 100};

    int i, sum = arr_sum(carray, sarray, array, larray);

    empty_byte(carray);
    empty_short(sarray);
    empty_word(array);
    empty_long(larray);

    sum += arr_sum(carray, sarray, array, larray);

    printf("Result: %d\n", sum);
    if (sum == 400) {
        printf("RESULT: PASS\n");
    } else {
        printf("RESULT: FAIL\n");
    }

    return sum;
}
