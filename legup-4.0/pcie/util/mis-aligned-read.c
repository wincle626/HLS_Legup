
#include <stdlib.h>
#include <stdio.h>
#include <riffa.h>
#include <assert.h>


int print(char *arg, int size)
{
    for (int i = 0; i < size; i++) {
        printf("%02d ", arg[i]);
    }
    printf("\n");
}

int main(int argc, char** argv) {

    int id = 0;
	fpga_t * fpga = fpga_open(0);
    const int chnl = 0;
    if (fpga == NULL) {
        fprintf(stderr, "ERROR: can't open fpga with id:%d\n", id);
        exit(-1);
    }

    int arg0[128] = {};
    int size = 8;

    char * buf = (char *)(arg0 + 1);
    printf("User base address: %p\n", buf);

    unsigned int rh[] = {
        0x20000000,
        size,
        0x0000ffff,
        0x30000000
    };

    // Send Request Header to initiate TX transfer
    int sent = fpga_send(fpga, chnl, &rh, sizeof(rh) / sizeof(int), -1, 1, 25000);
    assert(sent == 4);

    // Copy Data to user buffer
    int recv = fpga_recv(fpga, chnl, buf, size, 25000);

    print(buf, recv);

    return 0;
}
