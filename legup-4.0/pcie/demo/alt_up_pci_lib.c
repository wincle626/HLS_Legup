#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "alt_up_pci_lib.h"
#include "alt_up_pci_ioctl.h"

// global variable
int current_rw_bar_no = 0;

/**
 * This function is used to open the file, which represent your DE4 device.
 */
int alt_up_pci_open (int *fd, char *dev_file){

	// error checking
	if( fd == NULL || dev_file == NULL ){
		printf("Invalid input for alt_up_pci_open(). \n");
		return -1;
	}

	// Open the device file
	*fd = open(dev_file,O_RDWR);
	if( *fd < 0 ){
		printf("Could not open device\n");
		return -1;
	}
	
	return 0;
}

/**
 * This function used to close the file of the DE4 device.
 */
void alt_up_pci_close(int fd){
	close(fd);
}

/**
 * This function is used to do the read operation.
 */
int  alt_up_pci_read   (int fd, int bar, unsigned long addr, char *buff, unsigned long size){

	int retval; 
	struct alt_up_ioctl_arg handler;
	
	// error checking
	if( buff == NULL ){ 
		printf("Invalid input for alt_up_pci_read(). \n");
		return -1;
	}
		
	// set the BAR to read 
	if( current_rw_bar_no != bar ) {	
	
		handler.rw_bar_no = bar;
		retval = ioctl(fd, ALT_UP_IOCTL_SET_BAR, &handler);
		if( retval != 0 ){
			printf("ioctl() failed in alt_up_pci_read(). \n");
			return -1;
		}
	
		current_rw_bar_no = bar;
	}

	// set the file offset
	retval = lseek(fd, addr, SEEK_SET);
	if( retval < 0 ){
		printf("lseek() failed in alt_up_pci_read(). \n");
		return -1;
	}
			
	// perform read operation
	retval = read(fd, buff, size);
	if(retval < 0){
		printf("read() failed in alt_up_pci_read(). \n");
		return -1;
	} else if ( retval != size ){
		printf("read imcompletely.\n");
		return -1;
	}

	return 0;
}

/**
 * This function is used to do the write operation.
 */
int  alt_up_pci_write  (int fd, int bar, unsigned long addr, char *buff, unsigned long size){

	int retval;
	struct alt_up_ioctl_arg handler;
	
	// error checking
	if( buff == NULL ){ 
		printf("Invalid input for alt_up_pci_write(). \n");
		return -1;
	}
	
	// set the BAR to write 
	if( current_rw_bar_no != bar ){

		handler.rw_bar_no = bar;
		retval = ioctl(fd, ALT_UP_IOCTL_SET_BAR, &handler);
		if( retval != 0 ){
			printf("ioctl() failed in alt_up_pci_write(). \n");
			return -1;
		}
		
		current_rw_bar_no = bar;
	}

	// set the file offset
	retval = lseek(fd, addr, SEEK_SET); 
	if( retval < 0 ){
		printf("lseek() failed in alt_up_pci_write(). \n");
		return -1;
	}

	// perform write operation
	retval = write(fd, buff, size);
	if(retval < 0){
		printf("write() failed in alt_up_pci_write(). \n");
		return -1;
	} else if ( retval != size ){
		printf("write imcompletely.\n");
		return -1;
	}

	return 0;
}

/**
 * This function is used to add a DMA transaction into the DMA transaction queue.
 */
int  alt_up_pci_dma_add(int fd, int id,  unsigned long addr, char *buff, unsigned long length, int to_device){

	int retval;
	struct alt_up_ioctl_arg handler;

	// error checking
	if( buff == NULL ){ 
		printf("Invalid input for alt_up_pci_dma_add(). \n");
		return -1;
	}
	
	// set the struct alt_up_ioctl_arg 
	handler.dma_controller_id = id;
	handler.user_buffer_addr  = buff;	
	handler.dma_rw_slave_addr = addr;	
	handler.dma_length_byte   = length;
	handler.to_device         = to_device; 

	// perform the ioctl command
	retval = ioctl(fd, ALT_UP_IOCTL_DMA_ADD, &handler);
	if( retval != 0 ){
		printf("ioctl() failed in alt_up_pci_dma_add().\n");
		return -1;
	}

	return 0;
}

/**
 * This function is used to start the DMA transfer.
 */
int alt_up_pci_dma_go(int fd, int id, int use_interrupt){

	int retval;
	struct alt_up_ioctl_arg handler;

	// set the struct alt_up_ioctl_arg 
	handler.dma_controller_id = id;
	handler.use_interrupt = use_interrupt;

	// perform the ioctl command
	retval = ioctl(fd, ALT_UP_IOCTL_DMA_GO, &handler);
	if( retval != 0 ){
		printf("ioctl() failed in alt_up_pci_dma_go().\n");
		return -1;
	} 

	return 0;
}

