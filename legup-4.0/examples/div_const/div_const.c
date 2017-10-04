#include <stdio.h>

int main() {

    volatile unsigned a = 23534346, b = a;
    a /= 345345;
    b /= 641;
    volatile int c = 236343, d = c;
    c /= 734;
    d /= -351;
    printf("%d %d %d %d\n", a, b, c, d);
    // should equal: 68 + 36715 + 321 - 673
    int sum = a + b + c + d;
    printf("Result: %d\n", sum);
    if (sum == 36431) {
        printf("RESULT: PASS\n");
    } else {
        printf("RESULT: FAIL\n");
    }
    return sum;
}

