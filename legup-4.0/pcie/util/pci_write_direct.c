
#include <riffa.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <riffa_wrapper.h>

int
main (int argc, char * argv[]) {

    int arg[4] = {1, 1, 2, 3};
    pci_write_direct(arg, sizeof(arg), 0x20000000);

    arg[0] = 0x20000000;
    arg[1] = 3;
    pci_write_direct(arg, 4, 12);
    pci_write_direct(arg + 1, 4, 16);

    arg[0] = 1;
    pci_write_direct(arg, 4, 8);
    return 0;
}
