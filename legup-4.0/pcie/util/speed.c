
#include <riffa.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <riffa_wrapper.h>

#define K 1024
#define SIZE (128 * K)

#define ADDR 0x20000000
#define ITER 100000

char sendBuf[SIZE] = {};
char recvBuf[SIZE] = {};

int initBuf() {
    int i;
    for (i = 0; i < SIZE; i++) {
        sendBuf[i] = i;
    }
}

int verifyBuf(int size) {
    int i;
    for (i = 0; i < size; i++) {
        if (sendBuf[i] != recvBuf[i]) 
            printf("i = %d: %d doesn't match %d\n", i, sendBuf[i], recvBuf[i]);
    }
    return -1;
}

int
main (int argc, char * argv[]) {

    // initBuf();
    // assert( sizeof(sendBuf) == pcie_write(sendBuf, sizeof(sendBuf), ADDR));
    for (int i = 0; i < ITER; i++) {
        // int size = 1 + (i % SIZE);
        // assert( 4 == pcie_write(sendBuf, 4, ADDR + 8));
        // assert( 4 == pcie_write(sendBuf, 4, ADDR + 8));
        // assert( 4 == pcie_write(sendBuf, 4, ADDR + 8));
        // assert( 4 == pcie_write(sendBuf, 4, ADDR + 8));
        // assert( 4 == pcie_write(sendBuf, 4, ADDR + 8));
        // assert( 4 == pcie_read (recvBuf, 4, ADDR + 8));
        // assert( 4 == pcie_read (recvBuf, 4, ADDR + 8));
        // assert( 4 == pcie_read (recvBuf, 4, ADDR + 8));
        // assert( 4 == pcie_read (recvBuf, 4, ADDR + 8));
        // assert( 4 == pcie_read (recvBuf, 4, ADDR + 1000));
        // pcie_write(sendBuf, SIZE, ADDR);
        pcie_read (recvBuf, SIZE, ADDR);
        // assert(-1 == verifyBuf(SIZE));
        // printf("Iter %d done.\n", i);
    }
    // pcie_read (recvBuf, sizeof(recvBuf), ADDR);
    // assert(-1 == verifyBuf(SIZE));
    printf("buffer size = %d KB, iteration = %d\n", SIZE / K, ITER);
    printf("total transaction size =  %f MB\n", (double)SIZE * ITER / K / K);
    return 0;
}

