// Function declarations to simplify alt_up_pci_lib calls
void pci_init();
void pci_close();

// Write and read directly
void pci_read_direct(void *buf, size_t len, int offset);
void pci_write_direct(void *buf, size_t len, int offset);

// Write and read through the DMA
void pci_read_dma(void *buf, size_t len, int offset);
void pci_write_dma(void *buf, size_t len, int offset);
void pci_dma_go();
