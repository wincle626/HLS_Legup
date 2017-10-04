#include <stdio.h>

unsigned short lfsr = 0xACE1u;
unsigned bit;

#define N 20
int sortData[N];

unsigned my_rand() {
    bit = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5)) & 1;
    return lfsr = (lfsr >> 1) | (bit << 15);
}

void quickSort(int *arr, int elements) {
    int piv, beg[15], end[15], i = 0, L, R, swap;

    beg[0] = 0;
    end[0] = elements;
    while (i >= 0) {
        L = beg[i];
        R = end[i] - 1;
        if (L < R) {
            piv = arr[L];
            while (L < R) {
                while (arr[R] >= piv && L < R)
                    R--;
                if (L < R)
                    arr[L++] = arr[R];
                while (arr[L] <= piv && L < R)
                    L++;
                if (L < R)
                    arr[R--] = arr[L];
            }
            arr[L] = piv;
            beg[i + 1] = L + 1;
            end[i + 1] = end[i];
            end[i++] = L;
            if (end[i] - beg[i] > end[i - 1] - beg[i - 1]) {
                swap = beg[i];
                beg[i] = beg[i - 1];
                beg[i - 1] = swap;
                swap = end[i];
                end[i] = end[i - 1];
                end[i - 1] = swap;
            }
        } else {
            i--;
        }
    }
}

int main() {
    int i;

    for (i = 0; i < N; ++i) {
        sortData[i] = my_rand();
    }

    quickSort(sortData, N);

    // now check each array of 50 elements is indeed in sorted order
    int correct = 0;
    for (i = 1; i < N; i++)
        if (sortData[i] >= sortData[i - 1])
            correct++;

    printf("Result: %d\n", correct);

    if (correct == N - 1) {
        printf("RESULT: PASS\n");
    } else {
        printf("RESULT: FAIL\n");
    }
    return correct;
}
