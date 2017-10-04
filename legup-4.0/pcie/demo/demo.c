#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "alt_up_pci_lib_helper.h"

#define DATA_SIZE 750000
#define DATA_OFFSET 0x40000000

int main()
{
    char buf[DATA_SIZE], ref[DATA_SIZE];
    int i;

    srand(time(NULL));

    // initialize ref to random numbers
    for (i = 0; i < DATA_SIZE; i++) {
      // ensure no NULL characters
      ref[i] = (char)(rand() % 2 + 1);
    }

    memcpy(buf, ref, DATA_SIZE);

    pci_init();

    while (1) {
        struct timeval base_time;
        gettimeofday(&base_time, NULL);

        pci_write_dma(buf, DATA_SIZE, DATA_OFFSET);
        pci_dma_go();

        struct timeval curr_time;
        gettimeofday(&curr_time, NULL);

        long microseconds = curr_time.tv_usec - base_time.tv_usec + 1000000 * (curr_time.tv_sec - base_time.tv_sec);

        printf("Writing %d bytes took %ld microseconds\n", DATA_SIZE, microseconds);
        printf("\tWrite rate: %ld MB/s | ", (1000000 / microseconds * DATA_SIZE) >> 20);

        gettimeofday(&base_time, NULL);

        pci_read_dma(buf, DATA_SIZE, DATA_OFFSET);
        pci_dma_go();

        gettimeofday(&curr_time, NULL);

        microseconds = curr_time.tv_usec - base_time.tv_usec + 1000000 * (curr_time.tv_sec - base_time.tv_sec);

        printf("Reading %d bytes took %ld microseconds\n", DATA_SIZE, microseconds);
        printf("\tRead rate: %ld MB/s | ", (1000000 / microseconds * DATA_SIZE) >> 20);

        if (strncmp(ref, buf, DATA_SIZE)) {
          printf("Error: read data does not match\n");
          break;
        }
    }

    pci_close();

    return 0;
}
