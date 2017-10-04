/*
 * Copyright (c) 2013, Altera Corporation.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  - Redistributions of source code must retain the above copyright notice,
 *  this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright 
 * notice, this list of conditions and the following disclaimer in the 
 * documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

/* Top-level file for the driver.
 * Deal with device init and shutdown, BAR mapping, and interrupts. */

#include "aclpci.h"
#include "acl_init.c"
#include <asm/siginfo.h>    //siginfo
#include <linux/rcupdate.h> //rcu_read_lock
#include <linux/version.h>  //kernel_version


MODULE_AUTHOR  ("Dmitry Denisenko");
MODULE_LICENSE ("BSD"); // Taint the kernel
MODULE_DESCRIPTION ("Driver for Altera OpenCL Acceleration Boards");
MODULE_SUPPORTED_DEVICE ("Altera OpenCL Boards");



/* Use Message Signalled Interrupt (MSI).
 * If not used will get many visibly-distinct interrupts for a single
 * logical one (because it takes a while to reset the interrupt in the FPGA).
 * MSIs are faster. HOWEVER, currently seem to loose MSIs once in a while. :( */
#define USE_MSI 0

/* Static function declarations */
static int __devinit probe(struct pci_dev *dev, const struct pci_device_id *id);
static int __devinit init_chrdev (struct aclpci_dev *aclpci);
static void __devexit remove(struct pci_dev *dev);

static int __devinit scan_bars(struct aclpci_dev *aclpci, struct pci_dev *dev);
static int __devinit map_bars(struct aclpci_dev *aclpci, struct pci_dev *dev);
static void free_bars(struct aclpci_dev *aclpci, struct pci_dev *dev);


/* Populating kernel-defined data structures */
static struct pci_device_id aclpci_ids[] = {
  { PCI_DEVICE(ACL_PCI_ALTERA_VENDOR_ID, ACL_PCI_DE4_DEVICE_ID) },
  { PCI_DEVICE(ACL_PCI_ALTERA_VENDOR_ID, ACL_PCI_PCIE385_DEVICE_ID) },
  { 0 },
};
MODULE_DEVICE_TABLE(pci, aclpci_ids);


static struct pci_driver aclpci_driver = {
  .name = DRIVER_NAME,
  .id_table = aclpci_ids,
  .probe = probe,
  .remove = remove,
  /* resume, suspend are optional */
};


struct file_operations aclpci_fileops = {
  .owner =    THIS_MODULE,
  .read =     aclpci_read,
  .write =    aclpci_write,
/*  .ioctl =    aclpci_ioctl, */
  .open =     aclpci_open,
  .release =  aclpci_close,
};




/* Allocate /dev/BOARD_NAME device */
static int __devinit init_chrdev (struct aclpci_dev *aclpci) {

  int dev_minor =   0;
  int dev_major =   0; 
  int devno = -1;

  /* request major number for device */
  int result = alloc_chrdev_region(&aclpci->cdev_num, dev_minor, 1 /* one device*/, BOARD_NAME);
  dev_major = MAJOR(aclpci->cdev_num);
  if (result < 0) {
    ACL_DEBUG (KERN_WARNING "can't get major ID %d", dev_major);
    goto fail_alloc;
  }
  
  devno = MKDEV(dev_major, dev_minor);
    
  cdev_init (&aclpci->cdev, &aclpci_fileops);
  aclpci->cdev.owner = THIS_MODULE;
  aclpci->cdev.ops = &aclpci_fileops;
  result = cdev_add (&aclpci->cdev, devno, 1);
  /* Fail gracefully if need be */
  if (result) {
    printk(KERN_NOTICE "Error %d adding aclpci (%d, %d)", result, dev_major, dev_minor);
    goto fail_add;
  }
  ACL_DEBUG (KERN_DEBUG "aclpci = %d:%d", MAJOR(devno), MINOR(devno));
  /*
  aclpci->my_class = class_create(THIS_MODULE, "acl");
  if (IS_ERR(aclpci->my_class))
  {
    printk(KERN_NOTICE "Can't create class\n");
    goto fail_add;
  }

  aclpci->device = device_create (aclpci->my_class, NULL, aclpci->cdev_num, NULL, "acl");
  if (IS_ERR(aclpci->device))
  {
    printk(KERN_NOTICE "Can't create device\n");
    goto fail_dev_create;
  }
  */
  
  return 0;

 /*
fail_dev_create:
  class_unregister(aclpci->my_class);
  class_destroy(aclpci->my_class); */
  
/* ERROR HANDLING */
fail_add:
  /* free the dynamically allocated character device node */
  unregister_chrdev_region(devno, 1/*count*/);
  
fail_alloc:
  return -1;
}


/* Returns virtual mem address corresponding to location of IRQ control
 * register of the board */
static void* get_interrupt_enable_addr(struct aclpci_dev *aclpci) {

  /* Bar 2, register PCIE_CRA_IRQ_ENABLE is the IRQ enable register
   * (among other things). */
  return (void*)(aclpci->bar[ACL_PCI_CRA_BAR] + (unsigned long)PCIE_CRA_IRQ_ENABLE);
}


static void* get_interrupt_status_addr(struct aclpci_dev *aclpci) {

  /* Bar 2, register PCIE_CRA_IRQ_ENABLE is the IRQ enable register
   * (among other things). */
  return (void*)(aclpci->bar[ACL_PCI_CRA_BAR] + (unsigned long)PCIE_CRA_IRQ_STATUS);
}



/* Disable interrupt generation on the device. */
static void mask_irq(struct aclpci_dev *aclpci) {

  writel (0x0, get_interrupt_enable_addr(aclpci));
}


/* Enable interrupt generation on the device. */
static void unmask_irq(struct aclpci_dev *aclpci) {

  u32 val = 0;
  
#ifdef QSYS_IFACE
  val = ACL_PCIE_GET_BIT(ACL_PCIE_KERNEL_IRQ_VEC) | ACL_PCIE_GET_BIT(ACL_PCIE_DMA_IRQ_VEC);
#else
  val = ACL_PCIE_GET_BIT(PCIE_CRA_IRQ_RXMIRQ);
#endif

  writel (val, get_interrupt_enable_addr(aclpci));
}


//
// IDENTICAL COPY OF THIS FUNCTION IS IN HAL/PCIE.
// KEEP THE TWO COPIES IN SYNC!!!
//
// Given irq status, determine type of interrupt
// Result is returned in kernel_update/dma_update arguments.
// Using 'int' instead of 'bool' for returns because the kernel code
// is pure C and doesn't support bools.
void get_interrupt_type (unsigned int irq_status, 
                         unsigned int *kernel_update, unsigned int *dma_update)
{
#ifdef QSYS_IFACE
   *kernel_update = ACL_PCIE_READ_BIT( irq_status, ACL_PCIE_KERNEL_IRQ_VEC );
   *dma_update    = ACL_PCIE_READ_BIT( irq_status, ACL_PCIE_DMA_IRQ_VEC);
#else
   unsigned int irq_vector;
   irq_vector = ACL_PCIE_READ_BIT_RANGE( irq_status, PCIE_CRA_AVL_IRQ_VEC_HI, PCIE_CRA_AVL_IRQ_VEC_LO );

   // The vector is just garbage if the interrupt has already de-asserted
   if(ACL_PCIE_READ_BIT(irq_status, PCIE_CRA_IRQ_RXMIRQ))
   {
      *kernel_update = (irq_vector == ACL_PCIE_KERNEL_IRQ_VEC);
      *dma_update    = (irq_vector == ACL_PCIE_DMA_IRQ_VEC);
   }
#endif
}


#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 19)
irqreturn_t aclpci_irq (int irq, void *dev_id, struct pt_regs * not_used) {
#else
irqreturn_t aclpci_irq (int irq, void *dev_id) {
#endif


  struct aclpci_dev *aclpci = (struct aclpci_dev *)dev_id;
  u32 irq_status;
  irqreturn_t res;
  unsigned int kernel_update = 0, dma_update = 0;
  
 
  if (aclpci == NULL) {
    return IRQ_NONE;
  }
  
  /* During core reconfiguration, ignore interrupts. */
  if (aclpci->cvp_in_progress) {
    ACL_VERBOSE_DEBUG (KERN_WARNING "Ignoring interrupt while CVP is in progress");
    return IRQ_HANDLED;
  }
  
  /* From this point on, this is our interrupt. So return IRQ_HANDLED
   * no matter what (since nobody else in the system will handle this
   * interrupt for us). */
  aclpci->num_handled_interrupts++;
  
  /* If using old-style interrupts (dedicated wire), bring it down ASAP.
   * Otherwise, will get a flood of interrupts */
  #if !USE_MSI
    mask_irq(aclpci);
  #endif
  
  /* Can get interrupt for two reasons --  DMA descriptor processing is done
   * or kernel has finished. DMA is done entirely in the driver, so check for
   * that first and do NOT notify the user. */
  irq_status = readl ( get_interrupt_status_addr(aclpci) );
  
  get_interrupt_type (irq_status, &kernel_update, &dma_update);
  ACL_VERBOSE_DEBUG (KERN_WARNING "irq_status = 0x%x, kernel = %d, dma = %d",
                     irq_status, kernel_update, dma_update);

  if (kernel_update) {
    #if !POLLING
      /* Send SIGNAL to user program to notify about the kernel update interrupt. */
      if (aclpci->user_task != NULL) {
        int ret = send_sig_info(SIG_INT_NOTIFY, &aclpci->signal_info, aclpci->user_task);      
        if (ret < 0) {
          /* Can get to this state if the host is suspended for whatever reason.
           * Just print a warning message the first few times. The FPGA will keep
           * the interrupt level high until the kernel done bit is cleared (by the host).
           * See Case:84460. */
          aclpci->num_undelivered_signals++;
          if (aclpci->num_undelivered_signals < 5) {
            ACL_DEBUG (KERN_DEBUG "Error sending signal to host! irq_status is 0x%x\n", irq_status);
          }
        }
      }
    #else
       ACL_VERBOSE_DEBUG (KERN_WARNING "Kernel update interrupt. Letting host POLL for it.");
    #endif
    res = IRQ_HANDLED;
     
  }
  if (dma_update) {
    /* A DMA-status interrupt - let the DMA object handle this without going to
      * user space */
    res = aclpci_dma_service_interrupt(aclpci);
  
  } else {
    ACL_VERBOSE_DEBUG (KERN_WARNING "Our interrupt is neither for DMA nor for Kernel! irq_status is 0x%x\n", irq_status);
    res = IRQ_HANDLED;
  }
  
  
  /* Unmask board interrupts so can receive new ones */
  #if !USE_MSI
    unmask_irq(aclpci);
  #endif
  return res;
}


void load_signal_info (struct aclpci_dev *aclpci) {

  /* Setup siginfo struct to send signal to user process. Doing it once here
   * so don't waste time inside the interrupt handler. */
  struct siginfo *info = &aclpci->signal_info;
  memset(info, 0, sizeof(struct siginfo));
  info->si_signo = SIG_INT_NOTIFY;
  /* this is bit of a trickery: SI_QUEUE is normally used by sigqueue from user
   * space,  and kernel space should use SI_KERNEL. But if SI_KERNEL is used the
   * real_time data is not delivered to the user space signal handler function. */
  info->si_code = SI_QUEUE;
  info->si_int = 0;  /* Signal payload. Will be filled later with 
                        ACLPCI_CMD_SET_SIGNAL_PAYLOAD cmd from user. */
}


int init_irq (struct pci_dev *dev, void *dev_id) {

  u32 irq_type;
  struct aclpci_dev *aclpci = (struct aclpci_dev*)dev_id;
  int rc;

  if (dev == NULL || aclpci == NULL) {
    ACL_DEBUG (KERN_WARNING "Invalid inputs to init_irq (%p, %p)", dev, dev_id);
    return -1;
  }
  
  /* Message Signalled Interrupts. */  
  #if USE_MSI
    if(pci_enable_msi(dev) != 0){
      ACL_DEBUG (KERN_WARNING "Could not enable MSI");
    }
  #endif

  /* Do NOT use PCI_INTERRUPT_LINE config register. Its value is different
   * from dev->irq and doesn't work! Why? Who knows! */
  
  /* IRQF_SHARED   -- allow sharing IRQs with other devices */
  #if !USE_MSI 
    irq_type = IRQF_SHARED;
  #else
    /* No need to share MSI interrupts since they don't use dedicated wires.*/
    irq_type = 0;
  #endif
  
  rc = request_irq (dev->irq, aclpci_irq, irq_type, DRIVER_NAME, dev_id);
  if (rc) {
    ACL_DEBUG (KERN_WARNING "Could not request IRQ #%d, error %d", dev->irq, rc);
    return -1;
  }
  pci_write_config_byte(dev, PCI_INTERRUPT_LINE, dev->irq);
  ACL_VERBOSE_DEBUG (KERN_DEBUG "Succesfully requested IRQ #%d", dev->irq);
  
  aclpci->num_handled_interrupts = 0;
  aclpci->num_undelivered_signals = 0;
  
  aclpci_dma_init(aclpci);
  
  /* Enable interrupts */
  unmask_irq(aclpci);
  
  return 0;
}


void release_irq (struct pci_dev *dev, void *aclpci) {

  int i;
  int num_usignals;
  void* pio_out_addr_base;
  
  aclpci_dma_finish(aclpci);
  
  /* Disable interrupts before going away. If something bad happened in
   * user space and the user program crashes, the interrupt assigned to the device
   * will be freed (on automatic close()) call but the device will continue 
   * generating interrupts. Soon the kernel will notice, complain, and bring down
   * the whole system. */
  mask_irq(aclpci);
  
  ACL_VERBOSE_DEBUG (KERN_DEBUG "Freeing IRQ %d", dev->irq);
  free_irq (dev->irq, aclpci);
  
  ACL_VERBOSE_DEBUG (KERN_DEBUG "Handled %d interrupts", 
        ((struct aclpci_dev*)aclpci)->num_handled_interrupts);
        
  num_usignals = ((struct aclpci_dev*)aclpci)->num_undelivered_signals;
  if (num_usignals > 0) {
    ACL_DEBUG (KERN_DEBUG "Number undelivered signals is %d", num_usignals);
  }
    
  /* Perform software reset on the FPGA.
   * If the host is killed after launching a kernel but before the kernel
   * finishes, the FPGA will keep sending "kernel done" interrupt. That might
   * kill a *new* host before it can do anything. 
   *
   * WARNING: THIS RESET LOGIC IS ALSO IN THE HAL/PCIE.
   *          IF YOU CHANGE IT, UPDATE THE HAL AS WELL!!! */
  ACL_VERBOSE_DEBUG (KERN_DEBUG "Reseting kernel on FPGA");
  pio_out_addr_base = ((struct aclpci_dev*)aclpci)->bar[ACL_PCIE_PIO_OUT_BAR] + ACL_PCIE_PIO_OUT_OFFSET - ACL_PCIE_MEMWINDOW_SIZE;
  /* Do the reset */
  writel (ACL_PCIE_GET_BIT(PIO_OUT_SWRESET), pio_out_addr_base + PIO_SET);
  /* De-assert the reset */
  for (i = 0; i < 10; i++) {
    writel (ACL_PCIE_GET_BIT(PIO_OUT_SWRESET), pio_out_addr_base + PIO_CLR);
  }
  
  #if USE_MSI
    pci_disable_msi (dev);
  #endif
  mask_irq(aclpci);
}


/* Find upstream PCIe root node. 
 * Used for re-training and disabling AER. */
static struct pci_dev* find_upstream_dev (struct pci_dev *dev) {
  struct pci_bus *bus = 0;
  struct pci_dev *bridge = 0;
  struct pci_dev *cur = 0;
  int found_dev = 0;
  
  bus = dev->bus;
  if (bus == 0) {
    ACL_DEBUG (KERN_WARNING "Device doesn't have an associated bus!\n");
    return 0;
  }
  
  bridge = bus->self;
  if (bridge == 0) {
    ACL_DEBUG (KERN_WARNING "Can't get the bridge for the bus!\n");
    return 0;
  }
  
  ACL_DEBUG (KERN_DEBUG "Upstream device %x/%x, bus:slot.func %02x:%02x.%02x", 
             bridge->vendor, bridge->device,
             bridge->bus->number, PCI_SLOT(bridge->devfn), PCI_FUNC(bridge->devfn));
             
  ACL_DEBUG (KERN_DEBUG "List of downstream devices:");
  list_for_each_entry (cur, &bus->devices, bus_list) {
    if (cur != 0) {
      ACL_DEBUG (KERN_DEBUG "  %x/%x", cur->vendor, cur->device);
      if (cur == dev) {
        found_dev = 1;
      }
    }
  }
  
  if (found_dev) {
    return bridge;
  } else {
    ACL_DEBUG (KERN_WARNING "Couldn't find upstream device!");
    return 0;
  }
}

static int __devinit probe(struct pci_dev *dev, const struct pci_device_id *id) {

  struct aclpci_dev *aclpci = 0;
  int res;
  
  ACL_VERBOSE_DEBUG (KERN_DEBUG " probe (dev = 0x%p, pciid = 0x%p)", dev, id);
  ACL_DEBUG (KERN_DEBUG " vendor = 0x%x, device = 0x%x, class = 0x%x, bus:slot.func = %02x:%02x.%02x",
        dev->vendor, dev->device, dev->class, 
        dev->bus->number, PCI_SLOT(dev->devfn), PCI_FUNC(dev->devfn));

  acl_set_board_id_based_on_device_id (dev->device);
  if (board_id == -1) {
    ACL_DEBUG (KERN_WARNING "\nNO MAPPING FROM DEVICE ID to board_id!\n");
  }
  
  /* Load all board-related constants */
  acl_init_board_parameters();

  ACL_DEBUG (KERN_DEBUG "board_id is %d, ACL_PCI_CRA_BAR is %d, PCIE_CRA_IRQ_ENABLE is %d",
                        board_id, ACL_PCI_CRA_BAR, PCIE_CRA_IRQ_ENABLE);
  
  aclpci = kzalloc(sizeof(struct aclpci_dev), GFP_KERNEL);
  if (!aclpci) {
    ACL_DEBUG(KERN_WARNING "Couldn't allocate memory!\n");
    goto fail_kzalloc;
  }
  
  sema_init (&aclpci->sem, 1);
  aclpci->pci_dev = dev;
  dev_set_drvdata(&dev->dev, (void*)aclpci);
  aclpci->user_pid = -1;
  aclpci->cvp_in_progress = 0;
  aclpci->pci_gen = 0;
  aclpci->pci_num_lanes = 0;
  aclpci->upstream = find_upstream_dev (dev);
  
  retrain_gen2 (aclpci);
        
  aclpci->buffer = kmalloc (BUF_SIZE * sizeof(char), GFP_KERNEL);
  if (!aclpci->buffer) {
    ACL_DEBUG(KERN_WARNING "Couldn't allocate memory for buffer!\n");
    goto fail_kmalloc;
  }
  
  res = init_chrdev (aclpci);
  if (res) {
    goto fail_chrdev_init;
  }
  
  if (pci_enable_device(dev)) {
    ACL_DEBUG (KERN_WARNING "pci_enable_device() failed");
    goto fail_enable;
  }

  pci_set_master(dev);

  if (pci_request_regions(dev, DRIVER_NAME)) {
    goto fail_regions;
  }

  scan_bars(aclpci, dev);  
  if (map_bars(aclpci, dev)) {
    goto fail_map_bars;
  }

  return 0;


/* ERROR HANDLING */
fail_map_bars:
  pci_release_regions(dev);
  pci_disable_device (dev);
  
fail_regions:

fail_enable:
  unregister_chrdev_region (aclpci->cdev_num, 1);
  
fail_chrdev_init:
  kfree (aclpci->buffer);
  
fail_kmalloc:
  kfree (aclpci);
  
fail_kzalloc:
  return -1;
}



static int __devinit scan_bars(struct aclpci_dev *aclpci, struct pci_dev *dev)
{
  int i;
  for (i = 0; i < ACL_PCI_NUM_BARS; i++) {
    unsigned long bar_start = pci_resource_start(dev, i);
    if (bar_start) {
      unsigned long bar_end = pci_resource_end(dev, i);
      unsigned long bar_flags = pci_resource_flags(dev, i);
      ACL_DEBUG (KERN_DEBUG "BAR[%d] 0x%08lx-0x%08lx flags 0x%08lx",
         i, bar_start, bar_end, bar_flags);
    }
  }
  return 0;
}


/**
 * Map the device memory regions into kernel virtual address space
 * after verifying their sizes respect the minimum sizes needed, given
 * by the bar_min_len[] array.
 */
static int __devinit map_bars(struct aclpci_dev *aclpci, struct pci_dev *dev)
{
  int i;
  for (i = 0; i < ACL_PCI_NUM_BARS; i++){
    unsigned long bar_start = pci_resource_start(dev, i);
    unsigned long bar_end = pci_resource_end(dev, i);
    unsigned long bar_length = bar_end - bar_start + 1;
    aclpci->bar_length[i] = bar_length;

    if (!bar_start || !bar_end) {
      aclpci->bar_length[i] = 0;
      continue;
    }

    if (bar_length < 1) {
      ACL_DEBUG (KERN_WARNING "BAR #%d length is less than 1 byte", i);
      continue;
    }

    /* map the device memory or IO region into kernel virtual
     * address space */  
    aclpci->bar[i] = ioremap (bar_start, bar_length);

    if (!aclpci->bar[i]) {
      ACL_DEBUG (KERN_WARNING "Could not map BAR #%d.", i);
      return -1;
    }

    ACL_DEBUG (KERN_DEBUG "BAR[%d] mapped at 0x%p with length %lu.", i,
         aclpci->bar[i], bar_length);
  }
  return 0;
}  



static void free_bars(struct aclpci_dev *aclpci, struct pci_dev *dev) {

  int i;
  for (i = 0; i < ACL_PCI_NUM_BARS; i++) {
    if (aclpci->bar[i]) {
      pci_iounmap(dev, aclpci->bar[i]);
      aclpci->bar[i] = NULL;
    }
  }
}


static void __devexit remove(struct pci_dev *dev) {

  struct aclpci_dev *aclpci = 0;
  ACL_DEBUG (KERN_DEBUG ": dev is %p", dev);
  
  if (dev == 0) {
    ACL_DEBUG (KERN_WARNING ": dev is 0");
    return;
  }
  
  aclpci = (struct aclpci_dev*) dev_get_drvdata(&dev->dev);
  if (aclpci == 0) {
    ACL_DEBUG (KERN_WARNING ": aclpci_dev is 0");
    return;
  }
  
  cdev_del (&aclpci->cdev);
  unregister_chrdev_region (aclpci->cdev_num, 1);  
  free_bars (aclpci, dev);
  pci_disable_device(dev);
  pci_release_regions(dev);

  kfree (aclpci->buffer);
  kfree (aclpci);
}


/* Initialize the driver module (but not any device) and register
 * the module with the kernel PCI subsystem. */
static int __init aclpci_init(void) {

  ACL_DEBUG (KERN_DEBUG "----------------------------");
  ACL_DEBUG (KERN_DEBUG "Driver version: %s", ACL_DRIVER_VERSION);

  /* register this driver with the PCI bus driver */
  return pci_register_driver(&aclpci_driver);
}

static void __exit aclpci_exit(void)
{
  ACL_DEBUG (KERN_DEBUG "");

  /* unregister this driver from the PCI bus driver */
  pci_unregister_driver(&aclpci_driver);
}


module_init (aclpci_init);
module_exit (aclpci_exit);
