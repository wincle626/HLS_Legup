#define FLT2HEX(a) (*(unsigned*)&a)
#define DBL2HEX(a) (*(unsigned long long*)&a)

#include <stdio.h>

int main() {

    double array[3];
    array[0] = 1.2;
    array[1] = 2.2;
    array[2] = 3.3;

    double out = array[2];

    out = out / array[0];
    out = out / array[1];

    double ans = 1.25;

    printf("%x %x\n", DBL2HEX(ans), DBL2HEX(out));
    
    int ret_val;
    if (((out - ans) < 1e-6) || ((out - ans) > -1e-6))
        ret_val = 12;
    else 
        ret_val = -1;

    return ret_val;
}



