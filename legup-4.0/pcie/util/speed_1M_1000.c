

#include <riffa.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <riffa_wrapper.h>

#define K 1024

int sendBuf[1024 * 1024] = {};
int recvBuf[1024 * 1024] = {};

int initBuf() {
    int i;
    for (i = 0; i < 16 * K; i++) {
        sendBuf[i] = rand();
    }
}

int verifyBuf() {
    int i;
    for (i = 0; i < 16 * K; i++) {
        if (sendBuf[i] != recvBuf[i]) 
            return i;
    }
    return -1;
}
int
main (int argc, char * argv[]) {

    // initBuf();
    for (int i = 0; i < 1000; i++) {
        pci_write_direct(sendBuf, sizeof(sendBuf), 0x40000000);
        // pci_write_direct(sendBuf, 16, 0x40000000);
        // pci_read_direct(recvBuf, sizeof(recvBuf), 0x40000000);
        // printf("Iter %d done.\n", i);
        // delay
        // for (int j = 0; j < 100000; j++);
    }
    // assert(-1 == verifyBuf());
    return 0;
}

