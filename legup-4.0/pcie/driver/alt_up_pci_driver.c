/**
 *
 *
 * @file alt_up_pci_driver.c
 * @brief The source file contains all the functions defined in alt_up_pci_driver.h.
 */

#include "alt_up_pci_driver.h"
#include "alt_up_pci_ioctl.h"


#define RW_BUF_LEN 256

/**
 * This function is only called once, when the module in loaded into the kernel.
 * It will set the first entry of the pci_id_table to make the driver support
 * vendor_id and device_id set by the config_file (or 0x1172 and 0x0de4 by default).
 * Then, register the alt_up_pci_driver (struct pci_driver).
 *
 * @return Returns 0 on success. 
 *
 * @remark The current driver only support one set of vendor and device IDs. Changes 
 * need to be made to support multi-type of devices
 */
static int  __init alt_up_pci_init(void){

	// set the first entry of the pci_id_table
	alt_up_pci_id_table[0].vendor = vendor_id;
	alt_up_pci_id_table[0].device = device_id;
	alt_up_pci_id_table[0].subvendor = PCI_ANY_ID;
	alt_up_pci_id_table[0].subdevice = PCI_ANY_ID;

	printk(KERN_DEBUG "\n");
	printk(KERN_DEBUG DRV_NAME " init(), build at " __DATE__ " " __TIME__ "\n");

	return pci_register_driver(&alt_up_pci_driver);
}

/**
 * This function is only called once, when the module is removed from the kernel.
 * It will unregister the alt_up_pci_driver (struct pci_driver).
 */
static void __exit alt_up_pci_exit(void){

	printk(KERN_DEBUG "\n");
	printk(KERN_DEBUG DRV_NAME " exit(), build at " __DATE__ " " __TIME__ "\n");

	pci_unregister_driver(&alt_up_pci_driver);
}

/**
 * This function is called by the PCI core when it has a struct pci_dev that it 
 * thinks the driver wants to control. It will allocate the memory for the struct
 * alt_up_pci_dev, initialize it correctly and dynamically allocate a character
 * device node.
 *
 * @param[in] dev The pointer to the pci device that evokes the probe function.
 * @param[in] id  The pci_id_table of the driver.
 * 
 * @return Return 0 on success.
 */
static int  alt_up_pci_probe (struct pci_dev *dev, const struct pci_device_id *id) {

	int i, retval = 0;

	// allocate the memory for the struct alt_up_pci_dev
	struct alt_up_pci_dev *mydev = kmalloc( sizeof(struct alt_up_pci_dev), GFP_KERNEL );
	if (mydev == NULL){
		printk(KERN_DEBUG "kmalloc() memory for struct alt_up_pci_dev failed. \n");
		goto err_alloc_dev;
	}
	
	// save the pointers for the future usage
	pci_set_drvdata(dev, (void *)mydev);
	mydev->pci_dev = dev;
	
	// wake up the device             
	retval = pci_enable_device(dev);
	if (retval) {
		printk(KERN_DEBUG "pci_enable_device() failed. \n");
		goto err_enable_device;
	}

	// enables bus-mastering for device dev       
	pci_set_master(dev);
	
	// reserved PCI I/O and memory resources
	retval = pci_request_regions(dev, DRV_NAME);
	if (retval) {
		printk(KERN_DEBUG "pci_request_regions() failed. \n");
		goto err_request_regions;
	}
			
	// set the DMA addressing limitation
	retval = pci_set_dma_mask(dev, DMA_BIT_MASK( pci_dma_bit_range ));
	if (retval) {
		printk(KERN_DEBUG "pci_set_dma_mask() failed. \n");
		goto err_set_dma_mask;      
	}

	retval = pci_set_consistent_dma_mask(dev,DMA_BIT_MASK( pci_dma_bit_range ));
	if(retval) {
		printk(KERN_DEBUG "pci_set_consistent_dma_mask() failed. \n");
		goto err_set_dma_mask;
	}

	// set __iomem address, accessed by ioread, iowrite
	for (i = 0; i < MAX_NUM_OF_BARS; i ++) {
		if ( pci_resource_end(dev, i) != pci_resource_start(dev, i) ){

			/* create a virtual mapping cookie for a PCI BAR, 
			 * second arg is BAR, third is maxlen (0 means complete BAR) */
			mydev->bar[i] = pci_iomap(dev, i, 0); 
			if( !mydev->bar[i] ){
				printk(KERN_DEBUG "pci_iomap() failed. \n");
				goto err_iomap;
			}
			
			printk(KERN_DEBUG DRV_NAME " BAR%d initialized.\n", i);
			mydev->bar_size[i] = pci_resource_end(dev, i) - pci_resource_start(dev, i) + 1;
			
		} else  mydev->bar[i] = NULL;
	}

	// initialize the alt_up_pci_dev struct
	retval = alt_up_pci_dev_init(mydev);
	if(retval) {
		printk(KERN_DEBUG "alt_up_pci_dev_init() failed. \n");
		goto err_dev_init;
	}
	
	// have MSI enabled on its device function    
	retval = pci_enable_msi(dev);
	if (retval) {
		printk(KERN_DEBUG "pci_enable_msi() failed. \n");
		goto err_enable_msi;        
	}
			
	// request irq line for interrupt
	mydev->irq_line = dev->irq;
	retval = request_irq((int)mydev->irq_line, (void*)alt_up_pci_irqhandler, IRQF_SHARED, DRV_NAME, (void *)mydev);
	if (retval) {
		printk(KERN_DEBUG "pci_request_irq() failed. \n");
		goto err_request_irq;
	}

	// write irq_line to the PCI configuration space
	retval = pci_write_config_byte(dev, PCI_INTERRUPT_LINE, mydev->irq_line);
	if (retval) {
		printk(KERN_DEBUG "pci_read_config() failed. \n");
		goto err_write_config;       
	}  

	/* dynamically allocate a character device node
	 * 0 : requested minor
	 * 1 : count 
	 */
	retval = alloc_chrdev_region(&mydev->cdev_no, 0, 1, DRV_NAME);
	if(retval) {
		printk(KERN_DEBUG "alloc_chrdev_region() failed. \n");
		goto err_alloc_chrdev;
	}

	// init the cdev
	cdev_init(&mydev->cdev, &alt_up_pci_fops);
	mydev->cdev.owner = THIS_MODULE;
	mydev->cdev.ops = &alt_up_pci_fops;
	
	// add the cdev to kernel, from now on, the driver is alive
	retval = cdev_add(&mydev->cdev, mydev->cdev_no, 1);   /* 1: count */
	if(retval) {
		printk(KERN_DEBUG "cdev_add() failed. \n");
		goto err_cdev_add;
	}
	
	return 0;
	
	
	//cdev_del(&mydev->cdev);
err_cdev_add:
	unregister_chrdev_region(mydev->cdev_no, 1);  
err_alloc_chrdev:

err_write_config:
	free_irq(mydev->irq_line, (void *)mydev);
err_request_irq:
	pci_disable_msi(dev);
err_enable_msi:
	alt_up_pci_dev_exit(mydev);
err_dev_init:
	for (i = 0; i < MAX_NUM_OF_BARS; i ++) {
		if( mydev->bar[i] != NULL )
			pci_iounmap(dev, mydev->bar[i]);
	}
	goto err_set_dma_mask;
err_iomap:     
	for ( i = i - 1; i >= 0; i --){
		if( mydev->bar[i] != NULL)
			pci_iounmap(dev, mydev->bar[i]);
	}
err_set_dma_mask:
	pci_release_regions(dev);
err_request_regions:
	pci_disable_device(dev);
err_enable_device:
	kfree(mydev);
err_alloc_dev:
	printk("alt_up_pci_probe() failed with error: %d \n ", retval);

	return retval;        
}

/**
 * This function is called by the PCI core when the struct pci_dev is being removed 
 * from the system or the PCI driver is being unloaded from the kernel. It will undo
 * everything in alt_up_pci_probe()
 * 
 * @param[in] dev  The pointer to the pci device evoke the function
 */
static void alt_up_pci_remove(struct pci_dev *dev) {

	int i;
	struct alt_up_pci_dev *mydev = (struct alt_up_pci_dev *)pci_get_drvdata(dev);
		
	cdev_del(&mydev->cdev);
	unregister_chrdev_region(mydev->cdev_no, 1);
	free_irq( (int)mydev->irq_line, (void *)mydev);
	pci_disable_msi(dev);
	alt_up_pci_dev_exit(mydev);
	for (i = 0; i < MAX_NUM_OF_BARS; i ++){ 
		if( mydev->bar[i] != NULL )
			pci_iounmap(dev, mydev->bar[i]);
	}
	pci_release_regions(dev);
	pci_disable_device(dev);
	kfree(mydev);
}

/**
 * This function is called when the user application try to use the driver (by opening a file).
 * It retrieve the pointer to struct alt_up_pci_dev from the struct inode, and save it into struct
 * struct file to make the following file operations easier.
 *
 * @param[in] inode 
 * @param[in] filp
 * 
 * @return Return 0 on success
 *
 * @remark The current driver do not support multiple opening files. This function needs to be 
 * enhanced if supporting multiple opening files
 */
static int     alt_up_pci_open   (struct inode *inode, struct file *filp) {

	// get the pointer to the struct alt_up_pci_dev
	struct alt_up_pci_dev *mydev = container_of(inode->i_cdev, struct alt_up_pci_dev, cdev);
	
	// save the pointer into the struct file for future usage
	filp->private_data = (void *)mydev;
			
	return 0;     
}

/**
 * This function is called when the user application try to close the driver (by closing a file).
 * It should undo everything in alt_up_pci_open().
 *
 * @param[in] inode
 * @param[in] filp
 * 
 * @return Return 0 on success
 *
 * @remark The current driver do not support multiple opening files. This function needs to be 
 * enhanced if supporting multiple opening files
 */
static int     alt_up_pci_release(struct inode *inode, struct file *filp) {

	return 0;
}


/**
 * This function is called when the user application try to write to the file (by using write(fd, buf, count)). 
 * It will write "count" bytes to the "mydev->rw_bar_no" bar with a offset "*f_pos". If the count is multiple
 * of 4, it will use iowrite32(), otherwise iowrite8(). The performance of this function is slow, so it is
 * intended for small size data transfer. 
 * 
 * @param[in] filp
 * @param[in] buf   The pointer to the user buffer
 * @param[in] count The number of data in bytes needed to write 
 * @param[in] f_pos The current file position
 * 
 * @return Return the number of bytes written on success, -1 otherwise
 */
static ssize_t alt_up_pci_write  (struct file *filp, const char __user *buf, size_t count, loff_t *f_pos) {

	char *kernel_buf;
	ssize_t bytes_written = 0;
	struct alt_up_pci_dev *mydev = filp->private_data;
	char static_buf[RW_BUF_LEN];
	loff_t f_pos_val = *f_pos;

	// error checking
	if( f_pos_val + count > mydev->bar_size[mydev->rw_bar_no] ){
		printk(KERN_DEBUG "Trying to write to the outside of the BAR. \n");
		return -1;
	}
	
	// allocate the kernel buffer
	if( count <= RW_BUF_LEN ){
		kernel_buf = static_buf;
	} else{
		kernel_buf = kmalloc(count * sizeof(char), GFP_KERNEL);
	}

	// check whether the user buffer is readable
	if( access_ok(VERIFY_READ, (void __user *)buf, count) ) {
		// check the return value, if return not 0, copy imcompletely
		if( copy_from_user(kernel_buf, buf, count) ) {  
			printk(KERN_DEBUG "copy_from_user() failed. \n");
			return -1;                
		}
	} else{
		printk(KERN_DEBUG "access_ok() failed. \n");
		return -1;  
	}      
   
	// check whether count and file position are multiple of 4		
	if( (count % 4 == 0) && (f_pos_val % 4 == 0) ){
		while( count > 0 ){
			// write 32 bits each time
			iowrite32( ((u32 *)kernel_buf)[bytes_written/4], mydev->bar[mydev->rw_bar_no] + f_pos_val ); 
			wmb();
				
			count -= sizeof(u32);
			bytes_written += sizeof(u32);
			f_pos_val += sizeof(u32);     
		}     
	} else {	
		while( count > 0 ){
			// write 8 bits each time
			iowrite8( kernel_buf[bytes_written], mydev->bar[mydev->rw_bar_no] + f_pos_val ); 
			wmb();
				
			count -= sizeof(u8);
			bytes_written += sizeof(u8);
			f_pos_val += sizeof(u8);     
		}     
	}

	if( count > RW_BUF_LEN ){
		// free the kernel buffer after writing
		kfree(kernel_buf);
	}

	*f_pos = f_pos_val;
	
	// return the number of bytes written
	return bytes_written;
}

/**
 * This function is called when the user application try to read from the file (by using read(fd, buf, count)).
 * It will read "count" bytes from the "mydev->rw_bar_no" bar with a offset "*f_pos". If the count is multiple
 * of 4, it will use ioread32(), otherwise ioread8(). The performance of this function is slow, so it is
 * intended for small size data transfer. 
 *
 * @param[in] filp
 * @param[in] buf   The pointer to the user buffer
 * @param[in] count The number of data in bytes needed to be read 
 * @param[in] f_pos The current file position
 * 
 * @return Return the number of bytes written on success, -1 otherwise
 */
static ssize_t alt_up_pci_read   (struct file *filp, char __user *buf, size_t count, loff_t *f_pos){

	char *kernel_buf;
	ssize_t bytes_read = 0;
	struct alt_up_pci_dev *mydev = filp->private_data;
	char static_buf[RW_BUF_LEN];
	loff_t f_pos_val = *f_pos;

	// error checking
	if( f_pos_val + count > mydev->bar_size[mydev->rw_bar_no] ){
		printk(KERN_DEBUG "Trying to read from the outside of the BAR. \n");
		return -1;
	}	
	
	// allocate kernel buffer for reading
	if( count <= RW_BUF_LEN ){
		kernel_buf = static_buf;
	} else{
		kernel_buf = kmalloc(count * sizeof(char), GFP_KERNEL);
	}

	// check whether count and file position are multiple of 4
	if ( (count % 4 == 0) && (f_pos_val % 4 == 0) ){
		while( count > 0 ) {
			// read 32 bits each time
			((u32 *)kernel_buf)[bytes_read/4] = ioread32(mydev->bar[mydev->rw_bar_no] + f_pos_val); 
			rmb();
			
			count -= sizeof(u32);
			bytes_read += sizeof (u32);
			f_pos_val += sizeof(u32);     
		}
	}else {
		while( count > 0 ) {
			// read 8 bits each time
			kernel_buf[bytes_read] = ioread8(mydev->bar[mydev->rw_bar_no] + f_pos_val); 
			rmb();
			
			count -= sizeof(u8);
			bytes_read += sizeof (u8);
			f_pos_val += sizeof(u8);     
		}
	}

	*f_pos = f_pos_val;

	// check the user buffer writable
	if( access_ok(VERIFY_WRITE, (void __user *)buf, bytes_read) ) {
		// check the return value, if return not 0, copy imcompletely
		if( copy_to_user(buf, kernel_buf, bytes_read) ){   
			printk(KERN_DEBUG "copy_to_user() failed. \n");
			return -1;
		}
	} else{
		printk(KERN_DEBUG "access_ok() failed. \n");
		return -1;  
	}

	if( count > RW_BUF_LEN ){
		// free the buffer after reading
		kfree(kernel_buf);
	}

	// return the number for bytes read
	return bytes_read;    
}

/**
 * This function is usually used along with read/write to set the proper offset addr for them.
 *
 * @param[in] filp
 * @param[in] off     The offset
 * @param[in] whence  The starting position for the offset
 * 
 * @return Return the new file position on success.
 */
static loff_t  alt_up_pci_llseek (struct file *filp, loff_t off, int whence) {

	loff_t newpos;

	switch(whence) {
		case 0: /* SEEK_SET */ // set the off, starting from the beginning of the file
			newpos = off;
			break;
		case 1: /* SEEK_CUR */ // set the off, starting from the current file position
			newpos = filp->f_pos + off;
			break;
		case 2: /* SEEK_END */
			newpos = -1;  // should not be set outside the range of the BAR address
			break;
		default: /* can't happen */
			return -EINVAL;
	}
	if (newpos < 0)
		return -EINVAL;
				
	filp->f_pos = newpos;
	return newpos;
}

/**
 * This function will first copy the alt_up_ioctl_arg to the kernel, and then execute the command
 * accordingly. 
 * There are two commands are related to DMA : ALT_UP_IOCTL_DMA_ADD, and ALT_UP_IOCTL_DMA_GO.
 * The DMA related structures and functions are defined in alt_up_pci_dma.h, alt_up_pci_sdma.h, and 
 * alt_up_pci_sgdma.h. The struct alt_up_ioctl_arg and the commands are defined in file alt_up_pci_ioctl.h.
 *
 * @param[in] inode
 * @param[in] filp
 * @param[in] cmd   The command of the ioctl command
 * @param[in] arg   The pointer to the structure alt_up_ioctl_arg in the user space
 * 
 * @return Return 0 on success
 *
 * @remark The function provides the ability to perform various types hardware control in addition to 
 * read and write the device. Adding new command needs to change the file alt_up_pci_ioctl.h and this 
 * function accordingly. 
 */
static long alt_up_pci_ioctl_unlocked(struct file *filp, unsigned int cmd, unsigned long arg) {

	struct alt_up_pci_dev *mydev = (struct alt_up_pci_dev*)filp->private_data;
	struct alt_up_dma_ctrller *myctrller;
	struct alt_up_ioctl_arg handler;

	// verify arg in user process
	if( access_ok(VERIFY_READ,(void __user *)arg, _IOC_SIZE(cmd))) {
		unsigned long temp;
		temp = copy_from_user( &handler, (int __user *)arg, sizeof(handler) );
		// copy ioctl handler from user space to kernel
		if( temp ){
			return -EFAULT;	
		}
	} 
	else {
			return -EFAULT;
	}
	
	// respond to the command accordingly
	switch(cmd){  

		case ALT_UP_IOCTL_DMA_ADD: // add a new DMA transaction into the queue of the selected DMA controller
		
			// error checking
			if( handler.dma_controller_id < 0 || handler.dma_controller_id >= MAX_NUM_OF_DMAS ) {
				printk(KERN_DEBUG "Error: Invalid id %d for DMA controller. \n", handler.dma_controller_id);
				return -EFAULT;
			} else if( handler.to_device != 0 && handler.to_device != 1 ){
				printk(KERN_DEBUG "Error: Invalid parameter, to_device = %d. \n", handler.to_device);
				return -EFAULT;
			}
		
			// let 'myctrller' point to the selected DMA controller
			myctrller = &mydev->controllers[handler.dma_controller_id];
			
			// check whether the DMA controller exist
			if( myctrller->type == 0 ){
				printk(KERN_DEBUG "Error: DMA%d is not initialized. \n", myctrller->id);
				return -EFAULT;
			}

			spin_lock(&myctrller->write_lock);

			// check the room in the queue
			while( myctrller->write_index + 1 == myctrller->read_index
					|| (myctrller->write_index == DMA_MAX_QUEUE_NUM-1 && myctrller->read_index == 0) ){
				long temp;

			        spin_unlock(&myctrller->write_lock);

				// release the DMA queue
				mutex_lock(&myctrller->read_lock);
				temp = alt_up_pci_sgdma_go( myctrller, handler.use_interrupt );
				mutex_unlock(&myctrller->read_lock);

				if( temp != 0 ){
					return temp;
				}

			        spin_lock(&myctrller->write_lock);
			}
		
			// set the queue entry
			myctrller->dma_queue[myctrller->write_index].user_buffer_addr  = handler.user_buffer_addr;
			myctrller->dma_queue[myctrller->write_index].dma_rw_slave_addr = handler.dma_rw_slave_addr;
			myctrller->dma_queue[myctrller->write_index].length            = handler.dma_length_byte;
			myctrller->dma_queue[myctrller->write_index].to_device         = handler.to_device;

			// increase the number of the valid entries 
			myctrller->write_index++;
			if( myctrller->write_index == DMA_MAX_QUEUE_NUM ){
				myctrller->write_index = 0;
			}

			spin_unlock(&myctrller->write_lock);
			
			break;
		case ALT_UP_IOCTL_DMA_GO: // set GO for DMA transfer in the queue
		
			// error checking
			if( handler.dma_controller_id < 0 || handler.dma_controller_id >= MAX_NUM_OF_DMAS ) {
				printk(KERN_DEBUG "Error: Invalid id %d for DMA controller. \n", handler.dma_controller_id);
				return -EFAULT;
			} else if( handler.use_interrupt < 0 || handler.use_interrupt > 2 ){
				printk(KERN_DEBUG "Error: Invalid parameter, use_interrupt = %d. \n", handler.use_interrupt);
				return -EFAULT;
			}
		
			// let 'myctrller' point to the selected DMA controller
			myctrller = &mydev->controllers[handler.dma_controller_id];
		
			// check the type of the selected DMA controller
			if( myctrller->type == S_DMA ){
				long temp = 0;
				// check to see if there is anything in the queue to clear
				if( myctrller->read_index == myctrller->write_index ){
					return temp;
				}
				mutex_lock(&myctrller->read_lock);
				temp=alt_up_pci_sdma_go ( myctrller, handler.use_interrupt );
				mutex_unlock(&myctrller->read_lock);
				return temp;
			}
			else if( myctrller->type == SG_DMA ){
				long temp = 0;
				// check to see if there is anything in the queue to clear
				if( myctrller->read_index == myctrller->write_index ){
					return temp;
				}
				mutex_lock(&myctrller->read_lock);
				temp = alt_up_pci_sgdma_go( myctrller, handler.use_interrupt );
				mutex_unlock(&myctrller->read_lock);
				return temp;
			}
			else {
				printk(KERN_DEBUG "Error: DMA%d is not initialized. \n", myctrller->id);
				return -EFAULT;
			}
			
			break;
		case ALT_UP_IOCTL_SET_BAR: // set the read/write BAR number

			if( handler.rw_bar_no <= 0 || handler.rw_bar_no >= MAX_NUM_OF_BARS){
				printk(KERN_DEBUG "Error: invalid BAR number. \n");
				return -EFAULT;
			}else if( mydev->bar[handler.rw_bar_no] == NULL ){
				printk(KERN_DEBUG "Error: try to access the unintialized BAR. \n");
				return -EFAULT;
			} 
			
			mydev->rw_bar_no = handler.rw_bar_no;
				 
			break;
		case ALT_UP_IOCTL_SAVE_REG: // save state of pcie registers
			pci_save_state(mydev->pci_dev);
			break;
		case ALT_UP_IOCTL_RESTORE_REG: // restore state of pcie registers
			pci_restore_state(mydev->pci_dev);
			break;
		default: // not defined command

			printk(KERN_DEBUG "Unknown Ioctl command.\n");
			return -EFAULT;
	}
	return 0;
}


/**
 * This function will initialize the components in the struct alt_up_pci_dev.  
 * 
 * @param[in] mydev
 * 
 * @return Return 0 on success, -1 otherwise.
 */
static int alt_up_pci_dev_init(struct alt_up_pci_dev *mydev) {

	int i;
	struct alt_up_dma_ctrller *myctrller;

	// set default read/write BAR to 0
	mydev->rw_bar_no = 0;
	
	// init semaphore
	// sema_init(&mydev->sem, 1);

	for( i = 0; i < MAX_NUM_OF_DMAS; i ++){
	
		myctrller= &mydev->controllers[i];

		// check whether an DMA type exists
		if( dma_type[i] != 0  ){

			// init the DMA controller
			myctrller->id = i;
			myctrller->type = (enum dma_ctrller_type) dma_type[i];
			
			myctrller->dma_ctrl_base_addr = mydev->bar[ dma_ctrl_bar_no[i] ] + dma_ctrl_base_addr[i];
			myctrller->pcie_cra_base_addr = mydev->bar[ pcie_cra_bar_no ] + pcie_cra_base_addr;
			myctrller->pcie_tx_base_addr  = tx_base_addr;
			
			myctrller->read_index = 0;
			myctrller->write_index = 0;
			init_waitqueue_head(&myctrller->wait_queue);	
			myctrller->condition = -1;
			myctrller->irq = dma_irq_no[i];

			mutex_init(&myctrller->read_lock);
			spin_lock_init(&myctrller->write_lock);
			
			myctrller->pci_dev = mydev->pci_dev;
			
			// call the DMA initialization function
			if( myctrller->type == S_DMA ){
				alt_up_pci_sdma_init( myctrller, sdma_data_width[i] );
			}else if( myctrller->type == SG_DMA){
				alt_up_pci_sgdma_init( myctrller );
			}else{
				printk(KERN_DEBUG "Error: unknown type of dma\n");
				return -1;
			}			

			printk(KERN_DEBUG DRV_NAME " DMA%d initialized as type%d. \n", myctrller->id, myctrller->type);

		} else {
			myctrller->id = i;
			myctrller->type = 0;
		}
	}
	
	/* enable interrupt for PCIe core*/
	iowrite32(0xFFUL, mydev->bar[pcie_cra_bar_no] + pcie_cra_base_addr + CRA_CONTROL_REG);
	wmb();

	return 0;
}

/**
 * This function will call the cleanup function for DMAs to undo everything in alt_up_pci_dev_init().
 *
 * @param[in] mydev
 */
static void alt_up_pci_dev_exit(struct alt_up_pci_dev *mydev) {

	int i;
	struct alt_up_dma_ctrller *myctrller;

	for( i = 0; i < MAX_NUM_OF_DMAS; i ++){
	
		// set the pointer to myctrller
		myctrller= &mydev->controllers[i];
			
		if( myctrller->type == S_DMA ){
			alt_up_pci_sdma_exit ( myctrller );
		}else if( myctrller->type == SG_DMA){
			alt_up_pci_sgdma_exit( myctrller );
		}
	}	
}

/**
 * This function will check which DMA controller causes the interrupt and then wake up 
 * all the waiting processes on the wait_queue of that DMA controller. 
 *
 * @param[in] irq
 * @param[in] dev_id
 *
 * @return Return IRQ_HANDLED if the interrupt is handled, otherwise return IRQ_NONE.
 */
static irqreturn_t alt_up_pci_irqhandler(int irq, void *dev_id) {

	int i;
	struct alt_up_dma_ctrller *myctrller;
	struct alt_up_pci_dev *mydev = (struct alt_up_pci_dev*)dev_id;

	// read the status register of the PCIe IP Core to determine which irq caused the interrupt
	int check = ioread32( mydev->bar[pcie_cra_bar_no] + pcie_cra_base_addr + CRA_STATUS_REG);
	rmb();

	// check which controller issue the irq
	for ( i = 0; i < MAX_NUM_OF_DMAS; i++){
	
		myctrller = &mydev->controllers[i];

		if( ( check & ( 1 << myctrller->irq )) != 0 ){
			// set the correct condition
			myctrller->condition = myctrller->irq;

			// wake up the waiting processes
			wake_up(&myctrller->wait_queue);

			return IRQ_HANDLED;
		}
	}
	return IRQ_NONE;
}

