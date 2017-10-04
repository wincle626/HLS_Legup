// Function declarations to simplify alt_up_pci_lib calls
void pci_init();
void pci_close();

// Write and read directly
void pci_read_direct(void *buf, int len, int offset);
void pci_write_direct(void *buf, int len, int offset);

// Write and read through the DMA
void pci_read_dma(void *buf, int len, int offset);
void pci_write_dma(void *buf, int len, int offset);
void pci_dma_go();

// Write and read through the DMA to a specific DMA controller
void pci_dma_go_with_ctrller(int ctrller);
void pci_write_dma_with_ctrller(void *buf, int len, int offset, int ctrller);
void pci_read_dma_with_ctrller(void *buf, int len, int offset, int ctrller);
