
#ifndef RIFFA_WRAPPER_H
#define RIFFA_WRAPPER_H
#include <stdlib.h>

typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;
typedef unsigned long long uint64_t;

void pci_init();
void pci_close();

// Write and read directly
int pci_read_direct(void *buf, size_t len, int offset);
int pci_write_direct(void *buf, size_t len, int offset);

// Write and read directly
int pcie_read(void *buf, size_t len, int offset);
int pcie_write(uint8_t *buf, size_t len, int offset);

int pcie_read_direct_jenny(void *buf, size_t len, int offset);
// int pcie_write_16_aligned(void *buf, size_t len, int offset);
// int pcie_write_8_aligned(void *buf, int offset);
// int pcie_write_4_aligned(void *buf, int offset);
// int pcie_write_2_aligned(void *buf, int offset);
// int pcie_write_1_aligned(void *buf, int offset);
// int pcie_write_max_aligned(void *buf, size_t max_count, int offset);
// 
// int pcie_read_16_aligned(void *buf, size_t len, int offset);
// int pcie_read_8_aligned(void *buf, int offset);
// int pcie_read_4_aligned(void *buf, int offset);
// int pcie_read_2_aligned(void *buf, int offset);
// int pcie_read_1_aligned(void *buf, int offset);
// int pcie_read_max_aligned(void *buf, size_t max_count, int offset);

// Write and read through the DMA
void pci_read_dma(void *buf, size_t len, int offset);
void pci_write_dma(void *buf, size_t len, int offset);
void pci_dma_go();
#define USE_RIFFIA

#endif
