#include <stdio.h>
#include<stdlib.h>

// there are 10 golden test input vectors to perform a built-in self test of
// this circuit
#define N 10
volatile unsigned TEST_INPUTS[N][5] = {
{89384, 30887, 92778, 36916, 47794},
{85387, 60493, 16650, 41422, 2363},
{90028, 68691, 20060, 97764, 13927},
{80541, 83427, 89173, 55737, 5212},
{95369, 2568, 56430, 65783, 21531},
{22863, 65124, 74068, 3136, 13930},
{79803, 34023, 23059, 33070, 98168},
{61394, 18457, 75012, 78043, 76230},
{77374, 84422, 44920, 13785, 98538},
{75199, 94325, 98316, 64371, 38336}};

int main() {

    int i;
    unsigned sum = 0;

    // perform 5 divisions in parallel 10 times
    loop: for (i = 0; i < N; i++) {
        unsigned result = 
            TEST_INPUTS[i][0] / 23412 +
            TEST_INPUTS[i][1] / 45064 +
            TEST_INPUTS[i][2] / 2141 +
            TEST_INPUTS[i][3] / 3262 +
            TEST_INPUTS[i][4] / 3141;

        printf("Result %d: %d\n", i, result);
        sum += result;
    }

    // check the result
    printf("Sum: %d\n", sum);
    if (sum == 577) {
        printf("RESULT: PASS\n");
    } else {
        printf("RESULT: FAIL\n");
    }
    return sum;
}

