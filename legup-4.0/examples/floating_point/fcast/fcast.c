#define FLT2HEX(a) (*(unsigned*)&a)
#define DBL2HEX(a) (*(unsigned long long*)&a)

#include <stdio.h>

int main() {
    float test_flt = 3.1415f;
    double test_dbl = (double) test_flt;
    float flt_exp = 3.1415f;
    double dbl_exp = 3.1415;

    test_flt = (float) test_dbl;

    printf("Converted: %x, %llx\n", FLT2HEX(test_flt), DBL2HEX(test_dbl));

    if (((test_flt - flt_exp) < 1e-6) && ((test_flt - flt_exp) > -1e-6))
        if (((test_dbl - dbl_exp) < 1e-6) && ((test_dbl - dbl_exp) > -1e-6))
            return 88;

    printf("FAIL: Expected:  %x, %llx\n", FLT2HEX(flt_exp), DBL2HEX(dbl_exp));

    return -1;
}



