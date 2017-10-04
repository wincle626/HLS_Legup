#include <riffa.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <riffa_wrapper.h>

int
main (int argc, char * argv[]) {
    pci_init();

    int buf[8];
    int i = 0;

    printf("Accelerator: ");
    // for (i = 0; i < 8; i++) {
        pci_read_direct(&buf[i], 4, 0x00000008);
        printf("%08x ", buf[i]);
    //}
    printf("\n");

    printf("Memory: ");
    pci_read_direct(buf, sizeof(buf), 0x20000000);
    for (i = 0; i < 8; i++) {
        printf("%08x ", buf[i]);
    }
    printf("\n");
    return 0;
}
