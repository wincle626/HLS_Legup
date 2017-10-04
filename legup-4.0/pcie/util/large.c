

#include <riffa.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <riffa_wrapper.h>

#define K 1024

int sendBuf[1024 * 4] = {};
int recvBuf[1024 * 4] = {};

int initBuf() {
    int i;
    for (i = 0; i < 4 * K; i++) {
        sendBuf[i] = rand();
    }
}

int verifyBuf() {
    int i;
    for (i = 0; i < 4 * K; i++) {
        if (sendBuf[i] != recvBuf[i]) 
            return i;
    }
    return -1;
}

int
main (int argc, char * argv[]) {

    initBuf();
    pcie_write(sendBuf, sizeof(sendBuf), 0x20000000);
    // pcie_write(sendBuf, 16 * 4, 0x0000000);

    pcie_read(recvBuf, sizeof(recvBuf), 0x20000000);

    assert(-1 == verifyBuf());
    return 0;
}
