
#include <stdlib.h>
#include <stdio.h>
#include <riffa.h>
#include <assert.h>
#include <riffa_wrapper.h>


int print(char *arg, int size)
{
    for (int i = 0; i < size; i++) {
        printf("%02d ", arg[i]);
    }
    printf("\n");
}

int main(int argc, char** argv) {

    pci_init();

    char arg0[1024] = {
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
        11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
        21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
        31, 32
    };
    int size = 16;

    // printf("start reading on-chip memory.\n");
    // pcie_read_16_aligned(arg0, size, 0x20000000);
    // print(arg0, size);
    // return 0;

    // pcie_read_16_aligned(arg0, size, 0x00002000);
    // print(arg0, size);
    // return 0;
    // pcie_read_16_aligned(arg0, size, 0x00000010);
    // print(arg0, size);

    // size = 8;
    // for (int i = 0; i < 32; i += size) {
    //     pcie_read_8_aligned(arg0, i);
    //     print(arg0, size);
    // }

    // size = 4;
    // for (int i = 0; i < 32; i += size) {
    //     pcie_read_4_aligned(arg0, i);
    //     print(arg0, size);
    // }

    // size = 2;
    // for (int i = 0; i < 32; i += size) {
    //     pcie_read_2_aligned(arg0, i);
    //     print(arg0, size);
    // }


    // size = 1;
    // for (int i = 0; i < 32; i += size) {
    //     pcie_read_1_aligned(arg0, i);
    //     print(arg0, size);
    // }

    // for (int i = 0; i < 31; i ++) {
    //     printf("addr = %d: ", i);
    //     int recv = pcie_read_max_aligned(arg0, 32, i);
    //     print(arg0, recv);
    // }

    // for (int i = 1; i <= 32; i ++) {
    //     printf("i = %d: ", i);
    //     int recv = pcie_read_max_aligned(arg0, i, 0);
    //     print(arg0, recv);
    // }

    // for (int i = 0; i < 32; i++) {
    //     printf("i = %02d: ", i);
    //     int recv = pcie_read(arg0, i, 0);
    //     print(arg0, recv);
    // }

    // int recv = pcie_read(arg0, 32, 0x20000000);
    // print(arg0, recv);

    // int recv = pcie_read(arg0, 4, 12);
    // print(arg0, recv);
    // return 0;

    pcie_write(arg0, sizeof(arg0), 0x20000000);
    for (int k = 0; k < 16; k ++){
        char * buf = arg0 + k;
        printf("User base address: %p\n", buf);
        for (int i = 0; i < 32; i++) {
            printf("i = %02d: ", i);
            int recv = pcie_read(buf, 32, i + 0x20000000);
            print(buf, recv);

        }
    }

    return 0;
	// fpga_t * fpga;
    // int sendBuf[4] = {
    //     0x00000000, // addr
    //     0x00000008, // length
    //     0x0000ffff, // byteenable
    //     0x30000000  // small write opcode and byte enable
    // };
	// fpga = fpga_open(0);
	// int sent = fpga_send(fpga, 0, sendBuf, 4, -1, 1, 25000);
    // if (sent == 4) {
    //     printf("send seems completed.\n");
    // }
    // assert(sent == 4);
    // char recvBuf[8 * 4];
    // int recvd = fpga_recv(fpga, 0, recvBuf, 8, 25000);
    // if (recvd == 8) {
    //     printf("recv seems completed.\n");
    // }
    // assert(recvd == 8);
    // for(int i = 0; i < 8 * 4; i++)
    //     printf("%1x ", recvBuf[i]);
    // printf("\n");
    // fpga_close(fpga);
    // return 0;
}
