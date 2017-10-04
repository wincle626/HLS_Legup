/**
 * 
 * 
 * @file alt_up_pci_driver.h
 * @brief The header file for the linux pci driver.
 *
 * This file defines the name of the driver, initialization/cleanup functions for 
 * the module, PCI device structure, file operations, and all the input parameters 
 * of the driver. 
 *
 * @remark The file should not be included by any file other than "alt_up_pci_driver.c".  
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h> 
#include <linux/interrupt.h>

#include "alt_up_pci_device.h"

/** 
 * @brief define the name of the driver.
 *
 * This name will be shown in /proc/devices file if the driver probes correctly.
 * If this name is changed, the DRV_NAME variable in the shell script file 
 * "load_alt_up_pci_driver.sh" has to be changed accordingly.
 */
#define DRV_NAME "alt_up_pci"


///@brief The module initialization function, called once when 'insmod' the module. 
static int  __init alt_up_pci_init(void);
///@brief The module cleanup function, called once when 'rmmod' the module.
static void __exit alt_up_pci_exit(void);

// Macros that designate the module's initialization and cleanup functions
module_init(alt_up_pci_init);
module_exit(alt_up_pci_exit);


///@brief The probe function in the PCI driver.
static int	alt_up_pci_probe	(struct pci_dev *dev, const struct pci_device_id *id);
///@brief The remove function in the PCI driver.
static void	alt_up_pci_remove	(struct pci_dev *dev);

/** 
 * @brief The structure defines a list of the different types of PCI devices that the driver supports.
 *
 * The content of the structure are zeros at the beginning, and the IDs of the supported device, which 
 * are determined by the configuration file, will be added into this structure in the function 
 * alt_up_pci_init() at the load time.
 * 
 * @remark The last entry of the pci_id_table must be zero.
 */
static struct pci_device_id alt_up_pci_id_table[] = {
	{ 0,}, 	{ 0,}
};
MODULE_DEVICE_TABLE(pci, alt_up_pci_id_table);

///The structure describe the PCI driver to the PCI core. 
static struct pci_driver alt_up_pci_driver = {
	.name 		= DRV_NAME,					
	.id_table 	= alt_up_pci_id_table,
	.probe 		= alt_up_pci_probe,
	.remove 	= alt_up_pci_remove,
};


///@brief The open function for the device file.
static int     alt_up_pci_open	 (struct inode *inode, struct file *filp);
///@brief The release function for the device file.
static int     alt_up_pci_release(struct inode *inode, struct file *filp);
///@brief The function to handle the ioctl system calls.
static long     alt_up_pci_ioctl_unlocked  (struct file *filp, unsigned int cmd, unsigned long arg);
///@brief The function to send data to the device.
static ssize_t alt_up_pci_write  (struct file *filp, const char __user *buf, size_t count, loff_t *f_pos);
///@brief The function to retrieve data from the device.
static ssize_t alt_up_pci_read	 (struct file *filp, char __user *buf, size_t count, loff_t *f_pos);
///@brief The function to change the current read/write position.
static loff_t  alt_up_pci_llseek (struct file *filp, loff_t off, int whence);

/** 
 * @brief The structure defines the set of functions associated to the driver.
 * 
 * Linux will consider the driver as part of the file system, so that these 
 * functions are the interface between the user applications and the driver.
 */
static struct file_operations alt_up_pci_fops = {
	.owner		= THIS_MODULE,
	.open		= alt_up_pci_open,
	.release 	= alt_up_pci_release,
	.unlocked_ioctl = alt_up_pci_ioctl_unlocked, 
	.write		= alt_up_pci_write,
	.read 		= alt_up_pci_read,
	.llseek		= alt_up_pci_llseek,
};


///@brief The internal function to initilize the struct alt_up_pci_dev, called in alt_up_pci_probe()
static int  alt_up_pci_dev_init(struct alt_up_pci_dev *mydev);	
///@brief The internal function to cleanup the struct alt_up_pci_dev, called in alt_up_pci_remove()
static void alt_up_pci_dev_exit(struct alt_up_pci_dev *mydev);

///@brief The interrupt handler
static irqreturn_t alt_up_pci_irqhandler(int irq, void *dev_id);


/// The default vendor ID that the driver support
static int vendor_id = 0x1172;
/// The default device ID that the driver support
static int device_id = 0x0de4;

/// The base address of the tx slave in PCI Express IP Core 
static unsigned long tx_base_addr = 0x00000000;

/// The BAR connected to the CRA of PCI Express IP Core ( default is BAR0 )
static int pcie_cra_bar_no = 0;
/// The base address of the CRA slave in PCI Express IP Core
static unsigned long pcie_cra_base_addr = 0x00000000;

/**
 * @brief The number of bits can be used in address when allocating DMA buffers.
 *
 * The default value is 32, but it may need to be change according to the settings
 * of the translation table in PCI Express IP Core.
 */
static int pci_dma_bit_range = 32;

/// The types of the DMA controllers, 0 means no DMA
static int dma_type[MAX_NUM_OF_DMAS] = {0,0,0,0};
/// The BARs connected to the control slave of DMA controllers
static int dma_ctrl_bar_no[MAX_NUM_OF_DMAS] = {0,0,0,0};
/// The base addresses of the control slave of DMA controllers
static unsigned long dma_ctrl_base_addr[MAX_NUM_OF_DMAS] = {
	0x00000000, 	0x00000000, 	0x00000000, 	0x00000000
};
/// The IRQs of DMA controllers
static int dma_irq_no[MAX_NUM_OF_DMAS] = {0,0,0,0};

/// The data width of DMA transfers, specific data for DMA Controller only
static int sdma_data_width[MAX_NUM_OF_DMAS] = {4, 4, 4, 4};


// Macros to export the parameters, so that they are changable at the load time
module_param(vendor_id,         int,   S_IRUGO);
module_param(device_id,         int,   S_IRUGO);
module_param(tx_base_addr,      ulong, S_IRUGO);
module_param(pcie_cra_base_addr,ulong, S_IRUGO);
module_param(pcie_cra_bar_no,   int,   S_IRUGO);
module_param(pci_dma_bit_range, int,   S_IRUGO);

module_param_array(dma_type,          int,   NULL, S_IRUGO);
module_param_array(dma_ctrl_bar_no,   int,   NULL, S_IRUGO);
module_param_array(dma_ctrl_base_addr,ulong, NULL, S_IRUGO);
module_param_array(dma_irq_no,        int,   NULL, S_IRUGO);
module_param_array(sdma_data_width,   int,   NULL, S_IRUGO);


// Macros to provide information of the module
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Linux PCI Express Driver for Altera DE4 Boards");
