// Multi-dimensional arrays
// Expected result: 156

#include <stdio.h>
int array[2][2][3] = {{{1, 2, 3}, {4, 5, 6}}, {{7, 8, 9}, {10, 11, 12}}};

int fct(int *array, int size) {
    int result = 0;
    int i;
    for (i = 0; i < size; i++) {
        result += array[i];
    }
    return result;
}

int main() {
    int result = 0;
    int a, b, c;
    for (a = 0; a < 2; a++) {
        for (b = 0; b < 2; b++) {
            for (c = 0; c < 3; c++) {
                result += array[a][b][c];
            }
        }
    }

    result += fct((int *)array, 12);

    printf("Result: %d\n", result);
    if (result == 156) {
        printf("RESULT: PASS\n");
    } else {
        printf("RESULT: FAIL\n");
    }
    return result;
}
