#define FLT2HEX(a) (*(unsigned*)&a)
#define DBL2HEX(a) (*(unsigned long long*)&a)

#include <stdio.h>

int main() {
    int test = 5;

    float test_flt = (float) test;
    double test_dbl = (double) test;
    float flt_exp = 5;
    double dbl_exp = 5;

    if (((test_flt - flt_exp) < 1e-6) && ((test_flt - flt_exp) > -1e-6))
        if (((test_dbl - dbl_exp) < 1e-6) && ((test_dbl - dbl_exp) > -1e-6))
            return 88;

    printf("FAIL: Converted: %x, %llx\n", FLT2HEX(test_flt), DBL2HEX(test_dbl));
    printf("FAIL: Expected:  %x, %llx\n", FLT2HEX(flt_exp), DBL2HEX(dbl_exp));

    return -1;
}



