#define FLT2HEX(a) (*(unsigned*)&a)
#define DBL2HEX(a) (*(unsigned long long*)&a)

#include <stdio.h>

int main() {
    float flt = 5;
    double dbl = 5;

    int test_flt = (int) flt;
    int test_dbl = (int) dbl;
    int int_exp = 5;

    if (((test_flt - int_exp) < 1e-6) && ((test_flt - int_exp) > -1e-6))
        if (((test_dbl - int_exp) < 1e-6) && ((test_dbl - int_exp) > -1e-6))
            return 88;

    printf("FAIL: Converted: %x, %llx\n", FLT2HEX(test_flt), DBL2HEX(test_dbl));
    printf("FAIL: Expected:  %x, %llx\n", FLT2HEX(int_exp), DBL2HEX(int_exp));

    return -1;
}



