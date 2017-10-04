#define FLT2HEX(a) (*(unsigned*)&a)
#define DBL2HEX(a) (*(unsigned long long*)&a)

#include <stdio.h>

int main() {

    double ans1 = 3;
    double ans2 = 43;
    double ans3 = 0.68;

    double d1 = 1.1;
    double d2 = 3.3;
    double d3 = 0.1;
    double d4 = 4.3;
    double d5 = 2.5;
    double d6 = 1.7;

    double out1, out2, out3;

    out1 = d2/d1;
    out2 = d4/d3;
    out3 = d6/d5;

    if (((out1 - ans1) > 1e-6) || ((out1 - ans1) < -1e-6)){
        printf("FAIL: %x %x\n", DBL2HEX(out1), DBL2HEX(ans1));
        return -1;
    }
    printf("PASS: %x %x\n", DBL2HEX(out1), DBL2HEX(ans1));

    if (((out2 - ans2) > 1e-6) || ((out2 - ans2) < -1e-6)){
        printf("FAIL: %x %x\n", DBL2HEX(out2), DBL2HEX(ans2));
        return -1;
    }
    printf("PASS: %x %x\n", DBL2HEX(out2), DBL2HEX(ans2));

    if (((out3 - ans3) > 1e-6) || ((out3 - ans3) < -1e-6)){
        printf("FAIL: %x %x\n", DBL2HEX(out3), DBL2HEX(ans3));
        return -1;
    }
    printf("PASS: %x %x\n", DBL2HEX(out3), DBL2HEX(ans3));

    return 12;
}



