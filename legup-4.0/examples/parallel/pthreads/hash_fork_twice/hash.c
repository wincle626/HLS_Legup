#include <stdio.h>
#include <pthread.h>

#define SIZE 1800
#define BINSIZE SIZE

#include "hash.h"

#define NUM_ACCEL 4

void *hash1() {
    int result1[BINSIZE] = {0};
    // int result1[BINSIZE];
    int i, a, collision = 0;
    for (i = 0; i < SIZE; i++) {
        a = input[i];
        a = (a + 0x7ed55d16) + (a << 12);
        a = (a ^ 0xc761c23c) ^ (a >> 19);
        a = (a + 0x165667b1) + (a << 5);
        a = (a + 0xd3a2646c) ^ (a << 9);
        a = (a + 0xfd7046c5) + (a << 3);
        a = (a ^ 0xb55a4f09) ^ (a >> 16);
        a = (a < 0) ? -1 * a : a;
        int hash = a % BINSIZE;
        int temp = result1[hash]++;
        collision = (temp != 0) ? collision + 1 : collision;
    }
    pthread_exit((void *)collision);
}

void *hash2() {
    int result2[BINSIZE] = {0};
    // int result2[BINSIZE];
    int i, a, collision = 0;
    for (i = 0; i < SIZE; i++) {
        a = input[i];
        a -= (a << 6);
        a ^= (a >> 17);
        a -= (a << 9);
        a ^= (a << 4);
        a -= (a << 3);
        a ^= (a << 10);
        a ^= (a >> 15);
        a = (a < 0) ? -1 * a : a;
        int hash = a % BINSIZE;
        int temp = result2[hash]++;
        collision = (temp != 0) ? collision + 1 : collision;
    }
    pthread_exit((void *)collision);
}

void *hash3() {
    int result3[BINSIZE] = {0};
    // int result3[BINSIZE];
    int i, a, collision = 0;
    for (i = 0; i < SIZE; i++) {
        a = input[i];
        a = (a ^ 61) ^ (a >> 16);
        a = a + (a << 3);
        a = a ^ (a >> 4);
        a = a * 0x27d4eb2d;
        a = a ^ (a >> 15);
        a = (a < 0) ? -1 * a : a;
        int hash = a % BINSIZE;
        int temp = result3[hash]++;
        collision = (temp != 0) ? collision + 1 : collision;
    }
    pthread_exit((void *)collision);
}

void *hash4() {
    int result4[BINSIZE] = {0};
    // int result4[BINSIZE];
    int i, a, collision = 0;
    for (i = 0; i < SIZE; i++) {
        a = input[i];
        a = (a + 0x479ab41d) + (a << 8);
        a = (a ^ 0xe4aa10ce) ^ (a >> 5);
        a = (a + 0x9942f0a6) - (a << 14);
        a = (a ^ 0x5aedd67d) ^ (a >> 3);
        a = (a + 0x17bea992) + (a << 7);
        a = (a < 0) ? -1 * a : a;
        // printf("hash = %d\n", a);
        int hash = a % BINSIZE;
        int temp = result4[hash]++;
        collision = (temp != 0) ? collision + 1 : collision;
    }
    pthread_exit((void *)collision);
}

__attribute__((noinline)) void pthread_fork_1(pthread_t *threads,
                                              int *collision) {

    pthread_create(&threads[0], NULL, hash1, NULL);
    pthread_create(&threads[1], NULL, hash2, NULL);

    pthread_join(threads[0], (void **)&collision[0]);
    pthread_join(threads[1], (void **)&collision[1]);
}

__attribute__((noinline)) void pthread_fork_2(pthread_t *threads,
                                              int *collision) {

    pthread_create(&threads[2], NULL, hash3, NULL);
    pthread_create(&threads[3], NULL, hash4, NULL);

    pthread_join(threads[2], (void **)&collision[2]);
    pthread_join(threads[3], (void **)&collision[3]);
}

int main() {
    int i, j;
    int collision[NUM_ACCEL] = {0};
    pthread_t threads[NUM_ACCEL];

    // fork pthreads from two different functions
    pthread_fork_1(threads, collision);
    pthread_fork_2(threads, collision);

    int count = 0;
    printf("hash1 = %d, hash2 = %d, hash3 = %d, hash4 = %d\n", collision[0],
           collision[1], collision[2], collision[3]);
    count += (collision[0] == 1036);
    count += (collision[1] == 1035);
    count += (collision[2] == 1018);
    count += (collision[3] == 1028);
    printf("result = %d\n", count);
    if (count == 4) {
        printf("PASS\n");
    } else {
        printf("FAIL\n");
    }
    return 0;
}
