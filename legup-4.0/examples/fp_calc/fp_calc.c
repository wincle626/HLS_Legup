// a simple floating point multiplication example
// positive numbers

#define DBL2HEX(a) (*(unsigned *)&a)
#define fabs(a) ((a) > 0.0 ? (a) : -(a))	/* floating absolute value */

#include <stdio.h>
int main() {
    float a = 1.2345;
    float b = 2.3456;
    float c = 3.4567;
    float d = 4.5678;
    float e = 5.6789;
    float r1 = a * b;
    float r2 = (a - b) * c;
    float r3 = a * (b - c) / d;
    float r4 = (a + b) * (c * (d - e));
    float actual = r1 + r2 + r3 + r4;

    printf("%x * %x = %x\n", DBL2HEX(a), DBL2HEX(b), DBL2HEX(r1));
    printf("(%x - %x) * %x = %x\n", DBL2HEX(a), DBL2HEX(b), DBL2HEX(c),DBL2HEX(r2));
    printf("%x * (%x - %x) / %x = %x\n", DBL2HEX(a), DBL2HEX(b),DBL2HEX(c), DBL2HEX(d), DBL2HEX(r3));
    printf("(%x + %x) * (%x * (%x - %x)) = %x\n", DBL2HEX(a), DBL2HEX(b), DBL2HEX(c), DBL2HEX(d), DBL2HEX(e), DBL2HEX(r4));
    printf("%x + %x + %x + %x = %x\n", DBL2HEX(r1), DBL2HEX(r2), DBL2HEX(r3), DBL2HEX(r4), DBL2HEX(actual));

    float expected = -14.995611;
//    printf("Actual: %f\n", actual);
//    printf("Expected: %f\n", expected);
    printf("Actual:   %x\n", DBL2HEX(actual));
    printf("Expected: %x\n", DBL2HEX(expected));

    return 0;
}



