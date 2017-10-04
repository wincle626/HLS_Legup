// Left-shifting an integer by more than 32 bits
// Expected result: 496
//
// 1 << 200 
// = 1 << (200 % 32) 
// = 1 << 8
// = 256
// Taken from motion CHStone benchmark:
// 4042285200 >> 4294967128
// = 4042285200 >> (4294967128 % 32) 
// = 240

#include <stdio.h>
int main() {
    volatile unsigned int test = 1;
    volatile unsigned int N = 200;
    volatile int result = test << N;

    printf("1 << 200 = %d\n", result);
    printf("1 << (200 %% 32) = %d\n", test << (N % 32));

    test = 4042285200u;
    N = 4294967128u;

    volatile int result2 = test >> N;
    printf("%u >> %u = %u\n", test, N, result2);

    volatile int result_final = result + result2;

    printf("Result: %d\n", result_final);
    if (result_final == 496) {
        printf("RESULT: PASS\n");
    } else {
        printf("RESULT: FAIL\n");
    }

    return result_final;
}
