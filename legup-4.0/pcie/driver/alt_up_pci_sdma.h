/**
 * 
 * 
 * @file alt_up_pci_sdma.h
 * @brief The header file for the simple DMA controller.
 * 
 * This file defines parameters and functions for the DMA controllers.
 * You can change this file and alt_up_pci_sdma.c to improve the performance
 * of the DMA controller. 
 */
#ifndef __ALT_UP_PCI_SDMA_H__
#define __ALT_UP_PCI_SDMA_H__

#include "alt_up_pci_dma.h"

/// The size of the buffer allocated for the DMA transfer
#define SDMA_BUF_SIZE 4096
/// The number of cycles of checking repeated to cause a time out in polling mode
#define SDMA_POLLING_TIME_OUT_CYCLES 100000


/// The structure to save the private data for the sdma
struct sdma{
	/// the data width of the dma transfer
	int data_width_in_bytes;
	/// bit mask
	int dma_data_bit_mask;
	
	/// dma buffer kernel virtual addr
	void *virt_addr;
	/// dma buffer bus addr (physical addr)
	dma_addr_t bus_addr;
};

/// Offset of registers of the DMA controller
enum dma_register_map {
	DMA_REG_MAP_STATUS  = 0x0,
	DMA_REG_MAP_RD_ADDR = 0x4,
	DMA_REG_MAP_WT_ADDR = 0x8,
	DMA_REG_MAP_LEN     = 0xC,
	DMA_REG_MAP_CONTROL = 0x18
};

/// Bit mask for status registers
enum dma_status_bits {
	DMA_STATUS_DONE = 1 << 0,
	DMA_STATUS_BUSY = 1 << 1,
	DMA_STATUS_LEN  = 1 << 4
};

/// Bit mask for control registers
enum dma_control_bits {
	DMA_CTL_BIT_BYTE          = 1 << 0,
	DMA_CTL_BIT_HW            = 1 << 1,
	DMA_CTL_BIT_WORD          = 1 << 2,
	DMA_CTL_BIT_GO            = 1 << 3,
	DMA_CTL_BIT_I_EN          = 1 << 4,
	DMA_CTL_BIT_LEEN          = 1 << 7,
	DMA_CTL_BIT_DOUBLEWORD    = 1 << 10,
	DMA_CTL_BIT_QUADWORD      = 1 << 11,
	DMA_CTL_BIT_SOFTWARERESET = 1 << 12
};

/***************************************************************************************
 *              interface functions for simple DMA controller                          *
 ***************************************************************************************/

///@brief The function to initialize the DMA controller.
int  alt_up_pci_sdma_init(struct alt_up_dma_ctrller *myctrller, int transfer_data_width);

///@brief The function is called when the DMA controller is no longer used.
void alt_up_pci_sdma_exit(struct alt_up_dma_ctrller *myctrller);

///@brief The function to set go for DMA transfers.
int  alt_up_pci_sdma_go  (struct alt_up_dma_ctrller *myctrller, int use_interrupt);

///@brief The function to do the software reset.
inline void alt_up_pci_sdma_swreset(struct alt_up_dma_ctrller *myctrller);


#endif /* __ALT_UP_PCI_SDMA_H__ */
