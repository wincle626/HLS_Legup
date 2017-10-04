/**
 * 
 *
 * @file alt_up_pci_ioctl.h
 * @brief The header file defines the ioctl structure and ioctl commands.
 * 
 * Additional hardware controls can be added here. Remeber to change the 
 * function alt_up_pci_ioctl() accordingly, if any changes are made in 
 * this file. 
 */
#ifndef __ALT_UP_PCI_IOCTL_H__
#define __ALT_UP_PCI_IOCTL_H__ 

#include <linux/ioctl.h>

///structure used to pass information through ioctl commands
struct alt_up_ioctl_arg {

	/// The BAR to read from/write to in read and write functions
	int rw_bar_no;

	/// ID for DMA controller
	int dma_controller_id;

	/// parameters needed to set GO for DMA transaction(s)
	int use_interrupt;

	/// user buffer
	char *user_buffer_addr;
	/// the slave address to read/write
	unsigned long dma_rw_slave_addr;   
	/// length of the DMA transaction in bytes
	unsigned long dma_length_byte;
	/// direction of the DMA transaction
	int to_device; 
};

/// The magic number for ioctl
#define ALT_UP_IOC_MAGIC 'A'

/// The add dma ioctl command
#define ALT_UP_IOCTL_DMA_ADD      _IOWR(ALT_UP_IOC_MAGIC, 1, struct alt_up_ioctl_arg)
/// The go dma ioctl command
#define ALT_UP_IOCTL_DMA_GO       _IOWR(ALT_UP_IOC_MAGIC, 2, struct alt_up_ioctl_arg)
/// The set read/write BAR number
#define ALT_UP_IOCTL_SET_BAR      _IOWR(ALT_UP_IOC_MAGIC, 3, struct alt_up_ioctl_arg)
/// The save pcie register command
#define ALT_UP_IOCTL_SAVE_REG     _IOWR(ALT_UP_IOC_MAGIC, 4, struct alt_up_ioctl_arg)
/// The restore pcie register command
#define ALT_UP_IOCTL_RESTORE_REG  _IOWR(ALT_UP_IOC_MAGIC, 5, struct alt_up_ioctl_arg)

#endif /* __ALT_UP_PCI_IOCTL_H__ */
