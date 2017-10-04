/**
 *
 *
 * @file  alt_up_pci_sdma.c
 * @brief The source file for the simple DMA controller functions.
 *
 * The implementation of the funtions in this file is very basic and they are intended 
 * to provide an example of how to use the DMA controller. You will have to change them 
 * to improve the performance or satisfy your own need.  
 */

#include "alt_up_pci_sdma.h"

/***************************************************************************************
 *                                 internal functions                                  *
 ***************************************************************************************/
///@brief  The function to set the DMA controller.
static inline void alt_up_pci_sdma_set  (struct alt_up_dma_ctrller *myctrller, u32 local_addr, u32 phys_addr, u32 length, int to_device);

///@brief  The function to clear registers of DMA controller.
static inline void alt_up_pci_sdma_clear(struct alt_up_dma_ctrller *myctrller);

///@brief  The function to poll the result.
static int alt_up_pci_sdma_polling      (struct alt_up_dma_ctrller *myctrller);

///@brief  The function to use interrupt.
static int alt_up_pci_sdma_interrupt    (struct alt_up_dma_ctrller *myctrller);


/**
 * This function allocates buffer for the DMA controller to transfer data, initialize necessary data, 
 * and clear the registers for the controller.
 *
 * @param[in] myctrller            pointer to struct alt_up_dma_ctrller which points to the controller needed to be initialized
 * @param[in] transfer_data_width  data width setting for the DMA controller
 * 
 * @return  Returns 0 on success, -1 otherwise
 */
int  alt_up_pci_sdma_init(struct alt_up_dma_ctrller *myctrller, int transfer_data_width){
	
	// allocate memory to store data we need
	struct sdma *mydata = (struct sdma *) kmalloc( sizeof(struct sdma), GFP_KERNEL );
	
	// allocate buffer 
	mydata->virt_addr = (void *) pci_alloc_consistent(myctrller->pci_dev, SDMA_BUF_SIZE, &(mydata->bus_addr) );
	if( !(mydata->virt_addr) ) {
		printk(KERN_DEBUG "Error: allocate DMA buffer for DMA controller%d.\n" , myctrller->id);
		goto fail_alloc;
	}
	
	// set transfer_data_width
	mydata->data_width_in_bytes = transfer_data_width;
	
	// set bit mask
	switch(transfer_data_width) {
		case 1 : mydata->dma_data_bit_mask = DMA_CTL_BIT_BYTE ;       break;
		case 2 : mydata->dma_data_bit_mask = DMA_CTL_BIT_HW;          break;	
		case 4 : mydata->dma_data_bit_mask = DMA_CTL_BIT_WORD ;       break;
		case 8 : mydata->dma_data_bit_mask = DMA_CTL_BIT_DOUBLEWORD ; break;
		case 16: mydata->dma_data_bit_mask = DMA_CTL_BIT_QUADWORD ;   break;
		default: 
			printk(KERN_DEBUG "Error: not correct transfer_data_width %d\n", transfer_data_width);
			goto fail_data_width;
	};
	
	// save mydata
	myctrller->private_data = (void *)mydata;	

	// clear
	alt_up_pci_sdma_clear(myctrller);

	return 0;
	
fail_data_width:
	pci_free_consistent(myctrller->pci_dev, SDMA_BUF_SIZE, mydata->virt_addr, mydata->bus_addr);
fail_alloc:
	kfree(mydata);
	return -1;
}

/**
 * This function will free all the memory allocated in alt_up_pci_sdma_init().
 * The function is called in alt_up_pci_dev_exit().
 *
 * @param[in] myctrller  The DMA controller selected
 */
void alt_up_pci_sdma_exit(struct alt_up_dma_ctrller *myctrller){

	struct sdma *mydata = (struct sdma *) myctrller->private_data;

	// free the memory allocated
	pci_free_consistent(myctrller->pci_dev, SDMA_BUF_SIZE, mydata->virt_addr, mydata->bus_addr);
	kfree(mydata);
}

/**
 * This function will do all the transcations in the dma_queue. If the transfer size
 * is larger than the SDMA_BUF_SIZE, it will automatically divide the transfer into 
 * multiple tranfers.
 *
 * @param[in] myctrller      The DMA controller selected
 * @param[in] use_interrupt  if 1, set the DMA controller to assert irq signal
 *
 * @return  Returns 0 on success, -1 otherwise 
 */
int  alt_up_pci_sdma_go(struct alt_up_dma_ctrller *myctrller, int use_interrupt) { 

	int current_index, max_index = myctrller->write_index, auto_interrupt = (use_interrupt == 2);
	u32 bytes_left, chunk_bytes;
	u32 local_addr, controlReg;
	char *u_buf;
	
	struct alt_up_pci_dma *current_dma;
	struct sdma *mydata = (struct sdma *) myctrller->private_data;
	
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

		// check input for the DMA transfer
		if( bytes_left % mydata->data_width_in_bytes != 0){
			printk(KERN_DEBUG "Length is not suitable for data width\n");
			goto fail;
		} if(local_addr % mydata->data_width_in_bytes != 0){
			printk(KERN_DEBUG "Address is not aligned with data width\n"); 
			goto fail;
		} if(bytes_left == 0) {
			printk(KERN_DEBUG "Length is Zero\n");
			goto fail;
		}
		
		// The maximum transfer size is SDMA_BUF_SIZE
		// if DMA transfer size is larger than SDMA_BUF_SIZE, make multiple transfers
		while( bytes_left > 0 ){
		
			// set next transfer's chunk size
			chunk_bytes = bytes_left;
			if(chunk_bytes > SDMA_BUF_SIZE)
				chunk_bytes = SDMA_BUF_SIZE;
				
			if( auto_interrupt )
				use_interrupt = chunk_bytes >= (SDMA_BUF_SIZE / 4);

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
			alt_up_pci_sdma_set(myctrller, local_addr, (u32) mydata->bus_addr, chunk_bytes, current_dma->to_device);
		
			// set Go for DMA transaction 
			controlReg = DMA_CTL_BIT_GO | DMA_CTL_BIT_LEEN | mydata->dma_data_bit_mask | ( use_interrupt ? DMA_CTL_BIT_I_EN : 0x0UL) ;
			iowrite32( controlReg, myctrller->dma_ctrl_base_addr + DMA_REG_MAP_CONTROL);
			wmb(); 
					
			// wait for the transfer complete			
			if(use_interrupt){
				if( alt_up_pci_sdma_interrupt(myctrller) ){
					printk(KERN_DEBUG "alt_up_pci_sdma_interrupt() failed. \n");
					goto fail;
				}
			} else {
				if( alt_up_pci_sdma_polling(myctrller) ){
					printk(KERN_DEBUG "alt_up_pci_sdma_polling() failed. \n");
					goto fail;	
				}
			}
			
			// clear the status of the DMA registers
			alt_up_pci_sdma_clear(myctrller);
			
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
	alt_up_pci_sdma_swreset(myctrller);
	// empty the queue
	myctrller->read_index = max_index;
	printk(KERN_DEBUG "alt_up_pci_sdma_go() failed\n");	
	return -1;	
}


/**
 * This function will software reset the DMA controller.
 * You should use this function when the DMA transfer is stall,
 * and no other method can be used.
 *
 * @param[in] myctrller  The DMA controller selected
 */
inline void alt_up_pci_sdma_swreset(struct alt_up_dma_ctrller *myctrller){

	iowrite32( DMA_CTL_BIT_SOFTWARERESET, myctrller->dma_ctrl_base_addr + DMA_CTL_BIT_SOFTWARERESET );
	wmb();
	iowrite32( DMA_CTL_BIT_SOFTWARERESET, myctrller->dma_ctrl_base_addr + DMA_CTL_BIT_SOFTWARERESET );
	wmb();
}

/**
 * The function will write to the control register of DMA controller. It is important that you should 
 * add the pcie_tx_base_addr to the phys_addr to make sure the PCIe IP core works correctly.
 * 
 * @param[in] myctrller   The DMA controller selected
 * @param[in] phys_addr   The physical address of the kernel buffer
 * @param[in] local_addr  The avalon master address you want the DMA controller read from/write to
 * @param[in] length      Length of data to transfer in bytes
 * @param[in] to_device   if 1, transfer from PC to device; if 0, from device to PC
 * 
 * @remark  The dynamical translation table is not supported yet, you will have to change this function
 * to make the driver support it.
 **/
static inline void alt_up_pci_sdma_set(struct alt_up_dma_ctrller *myctrller, u32 local_addr, u32 phys_addr, u32 length, int to_device) {

	u32 rStartAddr = to_device ? phys_addr + myctrller->pcie_tx_base_addr : local_addr;
	u32 wStartAddr = to_device ? local_addr : phys_addr + myctrller->pcie_tx_base_addr;

	/* Config DMA controller */
	iowrite32( rStartAddr, myctrller->dma_ctrl_base_addr + DMA_REG_MAP_RD_ADDR);
	iowrite32( wStartAddr, myctrller->dma_ctrl_base_addr + DMA_REG_MAP_WT_ADDR);
	iowrite32( length,     myctrller->dma_ctrl_base_addr + DMA_REG_MAP_LEN); 
	wmb();
}

/**
 * This function clear the status and control registers after the DMA transfer finished.
 * The interrupt signal should be also cleared by this function.
 *
 * @param[in] myctrller  The DMA controller selected
 */
static inline void alt_up_pci_sdma_clear  (struct alt_up_dma_ctrller *myctrller){

	iowrite32( 0x00000000, myctrller->dma_ctrl_base_addr + DMA_REG_MAP_CONTROL);
	iowrite32( 0x00000000, myctrller->dma_ctrl_base_addr + DMA_REG_MAP_STATUS);
	wmb();
}

/**
 * This function implements polling method to check whether the DMA transfer finish.
 * It will keep polling the DONE bit of the status register of DMA controller.
 * 
 * @param[in] myctrller  The DMA controller selected
 */
static int alt_up_pci_sdma_polling  (struct alt_up_dma_ctrller *myctrller) {

	int i = 0, check = 0;
	void __iomem *dma_status_reg = myctrller->dma_ctrl_base_addr + DMA_REG_MAP_STATUS;
	
	while( (check & DMA_STATUS_DONE) ==0 ){
		// read the status register of the DMA controller
		check = ioread32( dma_status_reg );
		rmb();
		
		// check whether the number of times polling excede the SDMA_POLLING_TIME_OUT_CYCLES
		if( i++ > SDMA_POLLING_TIME_OUT_CYCLES ){
			printk(KERN_DEBUG "alt_up_pci_sdma_polling() time out. \n");
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
static int alt_up_pci_sdma_interrupt(struct alt_up_dma_ctrller *myctrller){
	
	long check_wait;
	int retval;
  
	// timeout at 1 second after
	check_wait = wait_event_timeout(myctrller->wait_queue, myctrller->condition == myctrller->irq, HZ);
	if(check_wait == 0) { // time out
		printk(KERN_DEBUG "alt_up_pci_sdma_interrupt() time out.\n");
		retval = -EIO;
	} else {
		retval = 0;
	}
	
	// set back the condition back to minus one
	myctrller->condition = -1;	
	
	return retval;
}
