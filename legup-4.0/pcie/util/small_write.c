
#include <stdlib.h>
#include <stdio.h>
#include <riffa.h>

int main(int argc, char** argv) {
	fpga_t * fpga;
    int sendBuf[4] = {
        0x12345678, // data
        0x9abcdeff, // data
        0x00000000, // addr
        0x1000000f  // small write opcode and byte enable
    };
	fpga = fpga_open(0);
	int sent = fpga_send(fpga, 0, sendBuf, 4, -1, 1, 25000);
    if (sent) {
        printf("seems completed.\n");
    }
    fpga_close(fpga);
    return 0;
}
