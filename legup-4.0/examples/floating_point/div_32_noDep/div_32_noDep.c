#define FLT2HEX(a) (*(unsigned*)&a)
#define DBL2HEX(a) (*(unsigned long long*)&a)

#include <stdio.h>

int main() {

    float ans1 = 3;
    float ans2 = 43;
    float ans3 = 0.68;

    float f1 = 1.1;
    float f2 = 3.3;
    float f3 = 0.1;
    float f4 = 4.3;
    float f5 = 2.5;
    float f6 = 1.7;

    float out1, out2, out3;

    out1 = f2/f1;
    out2 = f4/f3;
    out3 = f6/f5;

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



