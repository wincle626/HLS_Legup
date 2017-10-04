#define FLT2HEX(a) (*(unsigned*)&a)
#define DBL2HEX(a) (*(unsigned long long*)&a)

#include <stdio.h>

int main() {

    float array[3];
    array[0] = 1.1;
    array[1] = 2.2;
    array[2] = 3.3;

    float out = 0;

    out = out + array[0];
    out = out + array[1];
    out = out + array[2];

    float ans = 6.6;

    printf("%x %x\n", FLT2HEX(ans), FLT2HEX(out));
    
    int ret_val;
    if (((ans - out)<1e-6)&&((ans - out)>-1e-6))
        ret_val = 12;
    else 
        ret_val = -1;

    return ret_val;
}



