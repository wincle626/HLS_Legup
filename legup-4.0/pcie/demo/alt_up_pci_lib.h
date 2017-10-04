#ifndef __ALT_UP_PCI_LIB_H__
#define __ALT_UP_PCI_LIB_H__

enum BARs{
	BAR0 = 0,
	BAR1 = 1,
	BAR2 = 2,
	BAR3 = 3,
	BAR4 = 4,
	BAR5 = 5
};

enum Directions{
	FROM_DEVICE = 0,
	TO_DEVICE   = 1
};

enum {
	POLLING   = 0,
	INTERRUPT = 1,
        AUTO      = 2,
};

int  alt_up_pci_open   (int *fd, char *dev_file);
void alt_up_pci_close  (int fd);
int  alt_up_pci_read   (int fd, int bar, unsigned long addr, char *buff, unsigned long size);
int  alt_up_pci_write  (int fd, int bar, unsigned long addr, char *buff, unsigned long size);
int  alt_up_pci_dma_add(int fd, int id,  unsigned long addr, char *buff, unsigned long length, int to_device);
int  alt_up_pci_dma_go (int fd, int id,  int use_interrupt);

#endif    /* __ALT_UP_PCI_LIB_H__ */
