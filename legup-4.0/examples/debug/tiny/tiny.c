#include <stdio.h>

int main() {
    int sum = 0;
    int i;

    for (i = 0; i < 5; i++)
        sum += i;

    printf("Sum: %d\n", sum);
    return 0;
}
