#define HEX2FLT(a) (*(float*)&a)

#include <stdio.h>

int main() {
    int temp = 0x4f915d16;
    float f = HEX2FLT(temp);
    int d = (int)f;
    printf("Floating point: %f\n", f);
    printf("Signed decimal: %u\n", d);

    return d;
}



