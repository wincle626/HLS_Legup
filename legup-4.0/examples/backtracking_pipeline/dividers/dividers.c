#include <stdio.h>

#define FLT2HEX(a) (*(unsigned*)&a)
#define DBL2HEX(a) (*(unsigned long long*)&a)
#define fabs(a) (((a) >= 0.0f) ? (a) : -(a))

// 64 bit long long type
#define FIXEDPT_BITS 64

// 64 bits
// 7 bits for integer (1-100 range)
#define FIXEDPT_WBITS  7
// 57 bits for fraction
#define FIXEDPT_FBITS  57

#define I2F(a) (((TYPE)a)<<FIXEDPT_FBITS)
#define F2I(a) (((TYPE)a)>>FIXEDPT_FBITS)

#define TYPE unsigned long long



//#include "fixedptc.h"

#define N 40

#define INITIALIZE {1,2,3,4,5,6,7,8,9,10, 11,12,13,14,15,16,17,18,19,20, 21,22,23,24,25,26,27,28,29,30, 31,32,33,34,35,36,37,38,39,40, 41,42,43,44,45,46,47,48,49,50, 51,52,53,54,55,56,57,58,59,60, 61,62,63,64,65,66,67,68,69,70, 71,72,73,74,75,76,77,78,79,80, 81,82,83,84,85,86,87,88,89,90, 91,92,93,94,95,96,97,98,99,-100}

volatile TYPE c[100] = INITIALIZE;
volatile TYPE aa[100] = INITIALIZE;
volatile TYPE bb[100] = INITIALIZE;
volatile int cc[100] = INITIALIZE;
volatile TYPE dd[100] = INITIALIZE;

inline TYPE div(TYPE A, TYPE B)
{
    TYPE res = (((TYPE)A << FIXEDPT_FBITS) / (TYPE)B);
    printf("div = a/b = %lld/%lld = %lld\n", A, B, F2I(res));

    return res;
}


int main() {
    TYPE sum = 0;
    int i;
loop: for (i = 0; i < N; i++) {

          TYPE num = 100;
          TYPE den = 3;
          printf("test:\n");
          TYPE q = div(num, den);
          q = F2I(q);
          printf("test = 100/3 = %lld/%lld = %lld\n", num, den, q);

          printf("tmp7: ");
          TYPE tmp7 = div(aa[i+10], bb[i+1]>>1); 
          printf("tmp3: ");
          TYPE tmp3 = div(aa[i+1], bb[i+6]>>2); 
          printf("tmp2: ");
          TYPE tmp2 = div(dd[i+4], cc[i+7]>>3); 

          TYPE tmp5 = div(aa[i+8], cc[i+4]>>1); 


          int tmp8 = tmp2 + tmp5;
          int tmp9 = tmp3 + tmp7;
          int tmp10 = tmp8 + tmp9;

          cc[i+1] = F2I(tmp10 + c[i+1] + I2F(cc[i]));

      }

    sum = cc[i];
    printf("sum = %lld\n", sum);

    if (sum == 108) {
        printf("PASSED\n");
    } else {
        printf("FAILED\n");
    }

    return sum;
}
