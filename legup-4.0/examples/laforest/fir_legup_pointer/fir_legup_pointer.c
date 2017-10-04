
// 8-tap FIR filter, unrolled, in-memory
// References past values with pointer into input data, no buffer

// output will contain eight 1023 values, 
// and this sequence:
// 1024
// 1536
// 1792
// 1920
// 1984
// 2016
// 2032
// 2040
// 1020
// 510
// 255
// 127
// 63
// 31
// 15
// 7
// 3
// 1

#include <stdio.h>

// #define DEBUG

#define COUNT 100
#define TAPS  8

volatile int coefficients [TAPS] = {1,1,1,1,1,1,1,1};
int acc_temp            = 0;
int mul_temp            = 0;     

volatile int input [COUNT] = { 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 1023, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 1024, 512, 
256, 128, 64, 32, 16, 8, 4, 
2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0
};

volatile int output [COUNT] = {
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0 
};

int do_fir (volatile int *input) {
    acc_temp = input[-7] * coefficients[7];
    mul_temp = input[-6] * coefficients[6];
    acc_temp += mul_temp;
    mul_temp = input[-5] * coefficients[5];
    acc_temp += mul_temp;
    mul_temp = input[-4] * coefficients[4];
    acc_temp += mul_temp;
    mul_temp = input[-3] * coefficients[3];
    acc_temp += mul_temp;
    mul_temp = input[-2] * coefficients[2];
    acc_temp += mul_temp;
    mul_temp = input[-1] * coefficients[1];
    acc_temp += mul_temp;
    mul_temp = input[0] * coefficients[0];
    acc_temp += mul_temp;
    return acc_temp;
}

int main (void) {
  
  int i;

  // Measure this
  // Starts TAPS steps into the input to avoid indexing outside it.
 loop: for (i = TAPS; i < COUNT; i++) {
    output[i] = do_fir(&input[i]);
  }

#ifdef DEBUG       
    // Not this.
    for (i = 0; i < COUNT; i++) {
        printf("%d\n", output[i]);
    }
#endif
    
    return output[7];
}

