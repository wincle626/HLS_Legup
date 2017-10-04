
#include <riffa.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <riffa_wrapper.h>

int
main (int argc, char * argv[]) {

    char arg0[32] = {};
    char arg1[64] = {
        1, 2, 3, 4, 5, 6, 7,
        8, 9, 10, 11, 12, 13, 14, 15,
        16, 17, 18, 19, 20, 21, 22, 23,
        24, 25, 26, 27, 28, 29, 30, 31, 32
    };

    pci_init();
    int sent = pcie_write(arg1, 64, 0x20000000);
    // sent = pcie_write_16_aligned(arg0, 32, 0x4000000);

    // pcie_write_16_aligned(arg1, 16, 0x0000010);

    // int sent = pcie_write(arg1, 31, 0x0002001);
    printf("sent = %d\n", sent);

    // pcie_write_1_aligned(arg+10, 0x0000001);
    // pcie_write_1_aligned(arg+10, 0x0000003);
    // pcie_write_1_aligned(arg+10, 0x0000005);
    // pcie_write_1_aligned(arg+10, 0x0000007);
    // pcie_write_1_aligned(arg+10, 0x0000009);
    // pcie_write_1_aligned(arg+10, 0x000000b);
    // pcie_write_1_aligned(arg+10, 0x000000d);
    // pcie_write_1_aligned(arg+10, 0x000000f);
    // pcie_write_1_aligned(arg+10, 0x0000000);
    // pcie_write_1_aligned(arg+10, 0x0000002);
    // pcie_write_1_aligned(arg+10, 0x0000004);
    // pcie_write_1_aligned(arg+10, 0x0000006);
    // pcie_write_1_aligned(arg+10, 0x0000008);
    // pcie_write_1_aligned(arg+10, 0x000000a);
    // pcie_write_1_aligned(arg+10, 0x000000c);
    // pcie_write_1_aligned(arg+10, 0x000000e);
    // pcie_write_8_aligned(arg, 0x0000008);
    pci_close();

    return 0;
}
