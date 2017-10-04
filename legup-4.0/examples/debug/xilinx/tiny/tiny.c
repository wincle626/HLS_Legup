#include <stdio.h>

int main() {
    int sum = 0;
    int i;
    int A[10];

    for (i = 0; i < 10; i++)
        sum += A[i];

    printf("Sum: %d\n", sum);
    return 0;
}
