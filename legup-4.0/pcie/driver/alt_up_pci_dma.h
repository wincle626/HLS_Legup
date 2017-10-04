/**
 * 
 *
 * @file alt_up_pci_dma.h
 * @brief The header file defines the common interface for DMA controllers.
 *
 * Both DMA Controller and Scatter-Gather DMA Controller share the same interface.
 * It is recommended that any custom DMA controller should use this interface to make
 * the driver work properly. 
 */
#ifndef __ALT_UP_PCI_DMA_H__
#define __ALT_UP_PCI_DMA_H__

#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/mutex.h>
#include <asm/uaccess.h>

/**
 * Maximum number DMA transfer is allowed to put into the queue. 
 * By changing this, you can increase the maximum number of DMA transfer 
 * allowed in the queue.
 */
#define DMA_MAX_QUEUE_NUM 32

/// types of DMA controllers accepted by the driver
enum dma_ctrller_type {
	S_DMA   = 1,
	SG_DMA  = 2
};

/// The structure to represent the DMA transfer
struct alt_up_pci_dma {

	/// pointer to the user buffer
	char *user_buffer_addr;
	/// the slave address where you want to read/write to
	u32 dma_rw_slave_addr;
	/// transfer size in bytes
	u32 length;
	/// transfer direction, 1 -> transfer to device, 0 -> transfer to host
	int to_device;	
};

/// The structure to represent the DMA controller
struct alt_up_dma_ctrller{

	/// identifier for the DMA controller 
	int id;
	/// type of the DMA controller
	enum dma_ctrller_type type;

	/*************   Addresses    *************/
	/// ioR/W addr for the DMA controller
	void * __iomem dma_ctrl_base_addr;
	/// ioR/W addr for the CRA of the PCI Express IP core
	void * __iomem pcie_cra_base_addr;
	/// addr of the tx slave of the PCI Express IP core
	u32 pcie_tx_base_addr;
	
	/*************   Transfers    *************/
	/// the queue for the DMA transfers
	struct alt_up_pci_dma  dma_queue[DMA_MAX_QUEUE_NUM];
	/// the index of the start and end of the queue (equal means the queue is empty)
	int read_index, write_index;

	/*************   Interrupts   *************/
	/// wait_queue used for interrupt
	wait_queue_head_t wait_queue;
	/// when condition == irq, it means that the interrupt for this DMA controller transfer
	int condition;
	/// interrupt number for the DMA controller
	int irq;

	/*************    Locking     *************/
	/// locks for synchronization
	spinlock_t write_lock;
	struct mutex read_lock;

	/*************    Others      *************/
	/// pointer to pci_dev, needed to allocate memory
	struct pci_dev *pci_dev;
	/// private data for the DMA controller
	void *private_data;
};

#endif /*  __ALT_UP_PCI_DMA_H__  */
