#include <stdio.h>

int main() {
    volatile int x = 2;

    switch (x) {
    case 0:
        printf("case 0\n");
        return 1;
        break;
    case 1:
        printf("case 1\n");
        return 1;
        break;
    case 2:
        printf("case 2\n");
        printf("Result: 9\n");
        return 9;
        break;
    case 3:
        printf("case 3\n");
        return 1;
        break;
    default:
        return 2;
    }
}
