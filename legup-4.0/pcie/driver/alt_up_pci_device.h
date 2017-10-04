/**
 * 
 *
 * @file alt_up_pci_device.h
 * @brief The header file for structure and parameters related to the device. 
 *
 * This file defines the struct alt_up_pci_dev and parameters. It also includes 
 * the header files of different types of DMA controllers. 
 */
#ifndef __ALT_UP_PCI_DEVICE_H__
#define __ALT_UP_PCI_DEVICE_H__

#include <linux/cdev.h>

// Include two kinds of DMA controllers (DMA Controller and SG-DMA Controller)
#include "alt_up_pci_sdma.h"
#include "alt_up_pci_sgdma.h"

/// Define the maximum number of BARs supported
#define MAX_NUM_OF_BARS 6

/// Define the maximum number of DMA controllers supported.
#define MAX_NUM_OF_DMAS 4

/**
 * Offsets of some useful registers of the CRA slave of the PCI Express IP Core.
 * More information can be found in the IP Compiler for PCI Express User Guide.
 */
enum cra_register_map {
	CRA_STATUS_REG  = 0x40,
	CRA_CONTROL_REG = 0x50,
	CRA_TRANS_TABLE = 0x1000
};


/**
 * @brief The structure represents the device handled by the driver.
 * 
 * This structure contains the necessary information of the device. The current driver
 * only supports one device at a time, therefore only one struct alt_up_pci_dev will 
 * be instantiated.
 */
struct alt_up_pci_dev {

	/// pointer to pci_dev
	struct pci_dev *pci_dev;

	/// the current BAR number to read/write to
	int rw_bar_no;
	
	/// character device structure 
	struct cdev cdev;
	/// character device number
	dev_t cdev_no;
	
	/// array to record the io memory address for each BAR
	void * __iomem bar[MAX_NUM_OF_BARS];
	/// array to store the size of each BAR
	u32 bar_size[MAX_NUM_OF_BARS];

	/// interrupt request number of the device
	u8 irq_line;          

	/// DMA controllers of the device
	struct alt_up_dma_ctrller controllers[MAX_NUM_OF_DMAS];
};

#endif /* __ALT_UP_PCI_DEVICE_H__ */
