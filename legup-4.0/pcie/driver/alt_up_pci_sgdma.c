/**
 *
 *
 * @file alt_up_pci_sgdma.c
 * @brief The source file for the SG-DMA controller functions.
 *
 * The implementation of the funtions in this file is very basic and they are intended 
 * to provide an example of how to use the SG-DMA controller. You will have to change them 
 * to improve the performance or satisfy your own need. 
 */

#include "alt_up_pci_sgdma.h"

/***************************************************************************************
 *                                  internal functions                                 *
 ***************************************************************************************/
///@brief The function to set decriptors.
static void alt_up_pci_sgdma_setdesc     (struct alt_up_dma_ctrller *myctrller, u32 local_addr, u32 phys_addr, u32 length, int to_device);

///@brief The function to clear registers of SG-DMA controller.
static inline void alt_up_pci_sgdma_clear(struct alt_up_dma_ctrller *myctrller);

///@brief  The function to poll the result.
static int alt_up_pci_sgdma_polling      (struct alt_up_dma_ctrller *myctrller);

///@brief  The function to use interrupt.
static int alt_up_pci_sgdma_interrupt    (struct alt_up_dma_ctrller *myctrller);

/**
 * This function allocates buffer for the SG-DMA controller to transfer data, initialize necessary data, 
 * and clear the registers for the controller.
 *
 * @param[in] myctrller            pointer to struct alt_up_dma_ctrller which points to the controller needed to be initialized
 * 
 * @return  Returns 0 on success, -1 otherwise
 */
int  alt_up_pci_sgdma_init(struct alt_up_dma_ctrller *myctrller){

	// allocate memory to store data we need
	struct sgdma *mydata = (struct sgdma *) kmalloc( sizeof(struct sgdma), GFP_KERNEL );

	// allocate buffer 
	mydata->virt_addr = (void *) pci_alloc_consistent(myctrller->pci_dev, SGDMA_BUF_SIZE, &(mydata->bus_addr) );
	if( !(mydata->virt_addr) ) {
		printk(KERN_DEBUG "Error: allocate DMA buffer for DMA controller%d.\n" , myctrller->id);
		goto fail_alloc_buf;
	}
	
	// allocate sg_descriptor lists
	mydata->descriptors = (struct sg_descriptor *) pci_alloc_consistent(myctrller->pci_dev, SGDMA_DESCS_SIZE, &(mydata->descriptors_bus));
	if( !(mydata->descriptors) ){
		printk(KERN_DEBUG "Error: allocate DMA decriptors for DMA controller%d. \n",myctrller->id);
		goto fail_alloc_desc; 
	}

	// save mydata
	myctrller->private_data = (void *)mydata;
	
	// clear
	alt_up_pci_sgdma_clear(myctrller);
	
	return 0;
	
fail_alloc_desc:
	pci_free_consistent(myctrller->pci_dev, SGDMA_BUF_SIZE, mydata->virt_addr, mydata->bus_addr);	
fail_alloc_buf:
	kfree(mydata);
	return -1;
}

/**
 * This function will free all the memory allocated in alt_up_pci_sgdma_init().
 * The function is called in alt_up_pci_dev_exit().
 *
 * @param[in] myctrller  The DMA controller selected
 */
void alt_up_pci_sgdma_exit(struct alt_up_dma_ctrller *myctrller){

	struct sgdma *mydata = (struct sgdma *) myctrller->private_data;

	// free the memory allocated
	pci_free_consistent(myctrller->pci_dev, SGDMA_DESCS_SIZE, mydata->descriptors, mydata->descriptors_bus);
	pci_free_consistent(myctrller->pci_dev, SGDMA_BUF_SIZE, mydata->virt_addr, mydata->bus_addr);	
	kfree(mydata);	
}

/**
 * This function will do all the transcations in the dma_queue. If the transfer size
 * is larger than the SGDMA_BUF_SIZE, it will automatically divide the transfer into 
 * multiple tranfers.
 *
 * @param[in] myctrller      The DMA controller selected
 * @param[in] use_interrupt  if 1, set the DMA controller to assert irq signal
 *
 * @return  Returns 0 on success, -1 otherwise 
 */
int  alt_up_pci_sgdma_go(struct alt_up_dma_ctrller *myctrller, int use_interrupt) { 

	int current_index, max_index = myctrller->write_index, auto_interrupt = (use_interrupt == 2);
	u32 bytes_left, chunk_bytes;
	u32 local_addr, controlReg;
	char *u_buf;
	
	struct alt_up_pci_dma *current_dma;
	struct sgdma *mydata = (struct sgdma *) myctrller->private_data;

	for( current_index = myctrller->read_index; current_index != max_index; current_index++ ){
		if( current_index == DMA_MAX_QUEUE_NUM ){
			if( max_index == 0 ){
				break;
			}
			current_index = 0;
		}
		myctrller->read_index = current_index;
	
		// get infomation for this DMA transfer
		current_dma = &myctrller->dma_queue[current_index];
		u_buf      = current_dma->user_buffer_addr;
		local_addr = current_dma->dma_rw_slave_addr;
		bytes_left = current_dma->length;
	
		while( bytes_left > 0){
		
			// set next transfer's chunk size
			chunk_bytes = bytes_left;
			if( chunk_bytes > SGDMA_BUF_SIZE )
				chunk_bytes = SGDMA_BUF_SIZE;		

			if( auto_interrupt )
				use_interrupt = chunk_bytes >= (SGDMA_BUF_SIZE / 4);

			// if to_device == 1 then, copy data from user
			if ( current_dma->to_device == 1 ){ 
				if( access_ok(VERIFY_READ, (void __user *)u_buf, chunk_bytes) ){
					if( copy_from_user(mydata->virt_addr, u_buf, chunk_bytes) ) {
						printk(KERN_DEBUG "copy_from_user() failed. \n");
						goto fail;
					}
				} else {
				  printk(KERN_DEBUG "access check for copy_from_user() failed. \n");
				  goto fail;
				}

				// promise cache-coherency. cache-flush
				//dma_cache_sync(mydata->virt_addr, chunk_bytes, DMA_TO_DEVICE);  /* 2.6.30 older */
			}	
			
			// set DMA controller's registers
			alt_up_pci_sgdma_setdesc(myctrller, local_addr, (u32) mydata->bus_addr, chunk_bytes, current_dma->to_device);
			
			//
			iowrite32( (u32)mydata->descriptors_bus + myctrller->pcie_tx_base_addr, myctrller->dma_ctrl_base_addr + SGDMA_REG_MAP_NEXT_DESC );
			wmb();
			
			// set Go for DMA transaction
			controlReg = SGDMA_CTL_RUN | SGDMA_CTL_DESC_POLL_EN | (use_interrupt?(SGDMA_CTL_IE_CHAIN | SGDMA_CTL_IE_GLOBAL) : 0x0UL);
			iowrite32( controlReg, myctrller->dma_ctrl_base_addr + SGDMA_REG_MAP_CONTROL); 
			wmb();

			// wait for the transfer complete			
			if(use_interrupt){
				if( alt_up_pci_sgdma_interrupt(myctrller) ){
					printk(KERN_DEBUG "alt_up_pci_sgdma_interrupt() failed. \n");
					goto fail;
				}
			} else {
				if( alt_up_pci_sgdma_polling(myctrller) ){
					printk(KERN_DEBUG "alt_up_pci_sgdma_polling() failed. \n");
					goto fail;
				}
			}

			// clear the status of the DMA registers
			alt_up_pci_sgdma_clear(myctrller);

			// if to_device == 0 then, copy data to user
			if ( current_dma->to_device == 0 ) {
			
				// promise cache-coherency. cache-purge
				// dma_cache_sync(mydata->virt_addr, chunk_bytes, DMA_FROM_DEVICE);  	/* 2.6.30 older */			
			
				if( access_ok(VERIFY_WRITE, (void __user *)u_buf, chunk_bytes) ) {
					if( copy_to_user(u_buf, mydata->virt_addr, chunk_bytes) ){ 
						printk(KERN_DEBUG "copy_to_user() failed. \n");
						goto fail;
					}
				} else {
					printk(KERN_DEBUG "access check for copy_to_user() failed. \n");
					goto fail;
				}			
			}

			// update variables
			local_addr += chunk_bytes;
			u_buf      += chunk_bytes;
			bytes_left -= chunk_bytes;		
		}
	}
 
	// empty the queue
	myctrller->read_index = max_index;
 	return 0;

fail:
	alt_up_pci_sgdma_swreset(myctrller);
	// empty the queue
	myctrller->read_index = max_index;
	printk(KERN_DEBUG "alt_up_pci_sgdma_go() failed\n");	
	return -1;	
}

/**
 * This function will software reset the DMA controller.
 * You should use this function when the DMA transfer is stall,
 * and no other method can be used.
 *
 * @param[in] myctrller  The DMA controller selected
 */
inline void alt_up_pci_sgdma_swreset(struct alt_up_dma_ctrller *myctrller){

	iowrite32( SGDMA_CTL_SW_RESET , myctrller->dma_ctrl_base_addr + SGDMA_REG_MAP_CONTROL);
	wmb();	
	iowrite32( SGDMA_CTL_SW_RESET , myctrller->dma_ctrl_base_addr + SGDMA_REG_MAP_CONTROL);
	wmb();
}


/**
 * The function will set the descriptor table for the DMA controller. It is important that you should 
 * add the pcie_tx_base_addr to the phys_addr to make sure the PCIe IP core works correctly.
 * 
 * @param[in] myctrller   The DMA controller selected
 * @param[in] phys_addr   The physical address of the kernel buffer
 * @param[in] local_addr  The avalon master address you want the DMA controller read from/write to
 * @param[in] length      Length of data to transfer in bytes
 * @param[in] to_device   if 1, transfer from PC to device; if 0, from device to PC
 * 
 * @remark  The driver only use one descriptor of the table and the dynamical translation table is not supported yet.
 * You will have to change this function to make the driver support them. 
 */
static void alt_up_pci_sgdma_setdesc(struct alt_up_dma_ctrller *myctrller, u32 local_addr, u32 phys_addr, u32 length, int to_device){

	struct sgdma *mydata = (struct sgdma *) myctrller->private_data;
	
	u32 rStartAddr = to_device ? phys_addr + myctrller->pcie_tx_base_addr : local_addr;
	u32 wStartAddr = to_device ? local_addr : phys_addr + myctrller->pcie_tx_base_addr;

	mydata->descriptors[0].source            = rStartAddr;
	mydata->descriptors[0].destination       = wStartAddr;
	mydata->descriptors[0].next_desc_ptr     = (u32) (mydata->descriptors_bus + sizeof(struct sg_descriptor)) + myctrller->pcie_tx_base_addr;
	mydata->descriptors[0].bytes_to_transfer = length;
	mydata->descriptors[0].desc_control      = DESC_CTL_OWNED_BY_HW;
	
	mydata->descriptors[1].source            = 0;
	mydata->descriptors[1].destination       = 0;
	mydata->descriptors[1].next_desc_ptr     = 0;
	mydata->descriptors[1].bytes_to_transfer = 0;
	mydata->descriptors[1].desc_control      = 0;
}

/**
 * This function clear the status and control registers after the DMA transfer finished.
 * The interrupt signal should be also cleared by this function.
 *
 * @param[in] myctrller  The DMA controller selected
 */
static inline void alt_up_pci_sgdma_clear(struct alt_up_dma_ctrller *myctrller){

	iowrite32( SGDMA_CTL_CLEAR_INTR , myctrller->dma_ctrl_base_addr + SGDMA_REG_MAP_CONTROL);
	iowrite32( 0x00000000,            myctrller->dma_ctrl_base_addr + SGDMA_REG_MAP_STATUS);
	wmb();
}

/**
 * This function implements polling method to check whether the DMA transfer finish.
 * It will keep polling the BUSY bit of the status register of DMA controller.
 * 
 * @param[in] myctrller  The DMA controller selected
 */
static int alt_up_pci_sgdma_polling(struct alt_up_dma_ctrller *myctrller) {
	
	int i = 0, check = SGDMA_STATUS_BUSY;
	void __iomem *dma_status_reg = myctrller->dma_ctrl_base_addr + SGDMA_REG_MAP_STATUS;
	
	while( check & SGDMA_STATUS_BUSY || i < 5 /* determined by experimentation*/ ){
		// read the status register of the DMA controller
		check = ioread32( dma_status_reg );
		rmb();

		// check whether the number of times polling excede the SGDMA_POLLING_TIME_OUT_CYCLES
		if( i++ > SGDMA_POLLING_TIME_OUT_CYCLES ){
			printk(KERN_DEBUG "alt_up_pci_sgdma_polling() time out. \n");
			return -1;
		}
	}
	return 0;
}

/**
 * This function implements interrupt method to check whether the DMA transfer finish.
 * It will go to sleep until the interrupt handler wake up the process.
 *
 * @param[in] myctrller  The DMA controller selected
 */
static int alt_up_pci_sgdma_interrupt(struct alt_up_dma_ctrller *myctrller){

	long check_wait;
	int retval;

	/* timeout at 1 second after */
	check_wait = wait_event_timeout(myctrller->wait_queue, myctrller->condition == myctrller->irq, HZ);
	if(check_wait == 0) { /* time out */
		printk(KERN_DEBUG "alt_up_pci_sgdma_interrupt() time out.\n");
		retval = -EIO;
	} else {
		retval = 0;
	}
	
	// set back the condition back to minus one
	myctrller->condition = -1;
	
	return retval;
}
