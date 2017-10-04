#include <stdio.h>

#define OP1 11
#define OP2 4

int main() {

    volatile signed a = OP1, b = OP1, c = -OP1, d = -OP1, e = -OP1, f = -OP1;
    a /= -OP2;
    b %= -OP2;
    c /= OP2;
    d %= OP2;
    e /= -OP2;
    f %= -OP2;
    printf("%d %d %d %d %d %d\n", a, b, c, d, e, f);

    int result = (a == -2) + (b == 3) + (c == -2) + (d == -3) + (e == 2)
      + (f == -3);
    printf("Result: %d\n", result);
    if (result == 6) {
        printf("RESULT: PASS\n");
    } else {
        printf("RESULT: FAIL\n");
    }
    return result;
}

