/**
 * 
 *
 * @file alt_up_pci_sgdma.h
 * @brief The header file for the Scatter-Gather DMA controller.
 *
 * This file defines parameters and the functions for SG-DMA.
 * You can change this file and alt_up_pci_sdma.c to improve the performance
 * of the DMA controller. 
 */
#ifndef __ALT_UP_PCI_SGDMA_H__
#define __ALT_UP_PCI_SGDMA_H__

#include "alt_up_pci_dma.h"

/// The size of buffer for SG-DMA
#define SGDMA_BUF_SIZE 4096
/// The number of cycles of checking repeated to cause a time out in polling mode
#define SGDMA_POLLING_TIME_OUT_CYCLES 100000

/// The size of memory allocated to save the decriptors for SG-DMA
#define SGDMA_DESCS_SIZE 4096
/// The maximum number of decriptors supported
#define MAX_NUM_DESCS (SGDMA_DESCS_SIZE/(sizeof(sg_descriptor)))


/// descriptor for SG-DMA
struct sg_descriptor {
	/// source
	u32 source;            
	/// reserve
	u32 r0;
	/// destination          
	u32 destination;       
	/// reserve
	u32 r1;
	/// pointer to next descriptor               
	u32 next_desc_ptr;
	/// reserve      
	u32 r2;                 
	/// bytes to be transfered
	u16 bytes_to_transfer;
	/// reserve
	u16 r3;                 
	/// actual number of bytes transferred
	u16 actual_bytes_transferred;  
	///	status reg of descriptor
	u8  desc_status;              
	/// control reg of descriptor
	u8  desc_control;             
}__attribute__ ((packed));

/// bit mask for descriptor
enum desc_control_bits {
	DESC_CTL_GENERATE_EOP     = 1 << 0,
	DESC_CTL_READ_FIXED_ADDR  = 1 << 1,
	DESC_CTL_WRITE_FIXED_ADDR = 1 << 2,
	DESC_CTL_OWNED_BY_HW      = 1 << 7,
};

/// the structure to save the private data for the sgdma
struct sgdma{
	
	/// pointer to the sg-dma descriptor list
	struct sg_descriptor *descriptors;
	/// bus address for the descriptor
	dma_addr_t descriptors_bus;

	/// dma buffer kernel virtual addr
	void *virt_addr;
	/// dma buffer bus addr (physical addr)
	dma_addr_t bus_addr;
};

/// Offset for registers
enum sgdma_register_map {
	SGDMA_REG_MAP_STATUS    = 0x0,
	SGDMA_REG_MAP_CONTROL   = 0x10,
	SGDMA_REG_MAP_NEXT_DESC = 0x20
};

/// Bit mask for status register
enum sgdma_status_bits {
	SGDMA_STATUS_ERROR           = 1 << 0,
	SGDMA_STATUS_EOP_ENCOUNTERED = 1 << 1,
	SGDMA_STATUS_DESC_COMPLETED  = 1 << 2,
	SGDMA_STATUS_CHAIN_COMPLETED = 1 << 3,
	SGDMA_STATUS_BUSY            = 1 << 4,
};

/// Bit mask for control register
enum sgdma_control_bits {
	/**** interrupt ****/
	SGDMA_CTL_IE_ERROR   = 1 << 0,
	SGDMA_CTL_IE_EOP     = 1 << 1,
	SGDMA_CTL_IE_DESC    = 1 << 2,
	SGDMA_CTL_IE_CHAIN   = 1 << 3,
	SGDMA_CTL_IE_GLOBAL  = 1 << 4,

	SGDMA_CTL_RUN        = 1 << 5,
	SGDMA_CTL_STOP_ER    = 1 << 6,
	SGDMA_IE_MAX_DESC    = 1 << 7,

	SGDMA_CTL_SW_RESET      = 1 << 16,
	SGDMA_CTL_PARK          = 1 << 17,
	SGDMA_CTL_DESC_POLL_EN  = 1 << 18,
	SGDMA_CTL_CLEAR_INTR    = 1 << 31
};

/***************************************************************************************
 *              interface functions for Scatter-Gather DMA controller                          *
 ***************************************************************************************/

///@brief The function to initialize the SG-DMA controller.
int  alt_up_pci_sgdma_init(struct alt_up_dma_ctrller *myctrller);

///@brief The function is called when the SG-DMA controller is no longer used.
void alt_up_pci_sgdma_exit(struct alt_up_dma_ctrller *myctrller);

///@brief The function to set go for DMA transfers.
int  alt_up_pci_sgdma_go  (struct alt_up_dma_ctrller *myctrller, int use_interrupt);

///@brief The function to do the software reset.
inline void alt_up_pci_sgdma_swreset(struct alt_up_dma_ctrller *myctrller);

#endif /* __ALT_UP_PCI_SGDMA_H__ */
