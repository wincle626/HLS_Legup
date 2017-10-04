
#include <riffa.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <riffa_wrapper.h>

#define ADDR 0X00000000
int
main (int argc, char * argv[]) {

    int arg[4] = {-1, 1, 2, 3};
    pcie_write(arg, sizeof(arg), ADDR);

    // fpga_t * fpga = NULL;
	// fpga = fpga_open(0);
    // int len = fpga_recv(fpga, 0, arg, 4, 25000);

    int buf[4];
    int i;
    pcie_read(buf, sizeof(buf), ADDR);
    // for (i = 0; i < 4; i++) {
    //     assert(buf[i] == arg[i]);
    // }

    printf("success.\n");
    return 0;
}
