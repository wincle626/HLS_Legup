#define FLT2HEX(a) (*(unsigned*)&a)
#define DBL2HEX(a) (*(unsigned long long*)&a)

#include <stdio.h>

int main() {

    float ans1 = 3.63;
    float ans2 = 0.43;
    float ans3 = 4.59;

    float f1 = 1.1;
    float f2 = 3.3;
    float f3 = 0.1;
    float f4 = 4.3;
    float f5 = 2.7;
    float f6 = 1.7;

    float out1, out2, out3;

    out1 = f1*f2;
    out2 = f3*f4;
    out3 = f5*f6;

    if (((out1 - ans1) > 1e-6) || ((out1 - ans1) < -1e-6)){
        printf("FAIL: %x %x\n", FLT2HEX(out1), FLT2HEX(ans1));
        return -1;
    }
    printf("PASS: %x %x\n", FLT2HEX(out1), FLT2HEX(ans1));

    if (((out2 - ans2) > 1e-6) || ((out2 - ans2) < -1e-6)){
        printf("FAIL: %x %x\n", FLT2HEX(out2), FLT2HEX(ans2));
        return -1;
    }
    printf("PASS: %x %x\n", FLT2HEX(out2), FLT2HEX(ans2));

    if (((out3 - ans3) > 1e-6) || ((out3 - ans3) < -1e-6)){
        printf("FAIL: %x %x\n", FLT2HEX(out3), FLT2HEX(ans3));
        return -1;
    }
    printf("PASS: %x %x\n", FLT2HEX(out3), FLT2HEX(ans3));

    return 12;
}



