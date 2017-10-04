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


/* Implementation of all I/O functions except DMA transfers.
 * See aclpci_dma.c for DMA code.
 */

#include <linux/jiffies.h>
#include <linux/sched.h>
#include "aclpci.h"

static ssize_t aclpci_rw_large (void *dev_addr, void __user* use_addr, ssize_t len, char *buffer, int reading);



/* Given (bar_id, device_addr) pair, make sure they're valid and return
 * the resulting address. errno will contain error code, if any. */
void* aclpci_get_checked_addr (int bar_id, void *device_addr, size_t count,
                               struct aclpci_dev *aclpci, ssize_t *errno,
                               int print_error_msg) {

  if (bar_id >= ACL_PCI_NUM_BARS) {
    ACL_DEBUG (KERN_WARNING "Requested read/write from BAR #%d. Only have %d BARs!",
               bar_id, ACL_PCI_NUM_BARS);
    *errno = -EFAULT;
    return 0;
  }
  /* Make sure the final address is within range */
  if (((unsigned long)device_addr + count) > aclpci->bar_length[bar_id]) {
    if (print_error_msg) {
      ACL_DEBUG (KERN_WARNING "Requested read/write from BAR #%d from range (%lu, %lu). Length is %zu. BAR length is only %zu!",
                 bar_id, 
                 (unsigned long)device_addr,
                 (unsigned long)device_addr + count, 
                 count,
                 aclpci->bar_length[bar_id]);
    }
    *errno = -EFAULT;
    return 0;
  }

  *errno = 0;
  return (void*)(aclpci->bar[bar_id] + (unsigned long)device_addr);  
}


/* Compute address that contains memory window segment control */
static void *get_segment_ctrl_addr (struct aclpci_dev *aclpci) {

  void *dev_addr = 0;
  ssize_t errno = 0;
  void *ctrl_addr = (void*) (((ssize_t)ACL_PCIE_MEMWINDOW_CRA - ACL_PCIE_MEMWINDOW_SIZE) + 0);
  
  dev_addr = aclpci_get_checked_addr (ACL_PCIE_MEMWINDOW_BAR, ctrl_addr, sizeof(u64), aclpci, &errno, 1);
  if (errno != 0) {
    ACL_DEBUG (KERN_DEBUG "ERROR: ctrl_addr %p failed check", ctrl_addr);
    return NULL;
  }
  return dev_addr;
}


static void aclpci_set_segment_by_val (struct aclpci_dev *aclpci, u64 new_val) {

  void *ctrl_addr =  aclpci->global_mem_segment_addr;
  if (ctrl_addr == NULL) {
    return;
  }
  
  if (new_val != aclpci->global_mem_segment) {
    writeq (new_val, ctrl_addr);
    aclpci->global_mem_segment = new_val;
  }
  ACL_VERBOSE_DEBUG (KERN_DEBUG " Changed global memory segment to %llu.", new_val);
}


/* Response to user's open() call */
int aclpci_open(struct inode *inode, struct file *file) {

  struct aclpci_dev *aclpci = 0;
  int result = 0;

  /* pointer to containing data structure of the character device inode */
  aclpci = container_of(inode->i_cdev, struct aclpci_dev, cdev);
  
  if (down_interruptible(&aclpci->sem)) {
    return -ERESTARTSYS;
  }
  
  /* create a reference to our device state in the opened file */
  file->private_data = aclpci;
  ACL_DEBUG (KERN_DEBUG "aclpci = %p, pid = %d (%s)", 
             aclpci, current->pid, current->comm); 
  
  aclpci->user_pid = current->pid;
  aclpci->user_task = current;
  
  aclpci->global_mem_segment = 0;
  aclpci->global_mem_segment_addr = get_segment_ctrl_addr(aclpci);
#if 0
  if (aclpci->user_pid == -1) {
    aclpci->user_pid = current->pid;
  } else {
    ACL_DEBUG (KERN_WARNING "Tried open() by pid %d. Already opened by %d", current->pid, aclpci->user_pid);
    result = -EFAULT;
    goto done;
  }
#endif

  if (init_irq (aclpci->pci_dev, aclpci)) {
    ACL_DEBUG (KERN_WARNING "Could not allocate IRQ!");
    result = -EFAULT;
    goto done;
  }

  load_signal_info (aclpci);
  #if !POLLING
    if (aclpci->user_task == NULL) {
      ACL_DEBUG (KERN_WARNING "Tried open() by pid %d but couldn't find associated task_info", current->pid);
      result = -EFAULT;
      goto done;
    }
  #endif
  
  result = 0;
  
done:
  up (&aclpci->sem);
  return result;
}


/* Response to user's close() call. Will also be called by the kernel
 * if the user process dies for any reason. */
int aclpci_close(struct inode *inode, struct file *file) {

  ssize_t result = 0;
  struct aclpci_dev *aclpci = (struct aclpci_dev *)file->private_data;
  ACL_DEBUG (KERN_DEBUG "aclpci = %p, pid = %d, dma_idle = %d",
             aclpci, current->pid, aclpci_dma_get_idle_status(aclpci)); 
  
  if (down_interruptible(&aclpci->sem)) {
    return -ERESTARTSYS;
  }
  
#if 0  
  if (aclpci->user_pid == current->pid) {
    aclpci->user_pid = -1;
  } else {
    ACL_DEBUG (KERN_WARNING "Tried close() by pid %d. Opened by %d", current->pid, aclpci->user_pid);
    result = -EFAULT;
    goto done;
  }
#endif

  release_irq (aclpci->pci_dev, aclpci);

  up (&aclpci->sem);
  return result;
}


/* Read a small number of bytes and put them into user space */
ssize_t aclpci_read_small (void *read_addr, void __user* dest_addr, ssize_t len) {

  ssize_t copy_res = 0;
  switch (len) {
  case 1: {
    u8 d = readb ( read_addr );
    copy_res = copy_to_user ( dest_addr, &d, sizeof(d) );
    break;
  }
  case 2: {
    u16 d = readw ( read_addr );
    copy_res = copy_to_user ( dest_addr, &d, sizeof(d) );
    break;
  }
  case 4: {
    u32 d = readl ( read_addr );
    copy_res = copy_to_user ( dest_addr, &d, sizeof(d) );
    break;
  }
  case 8: {
    u64 d = readq ( read_addr );
    copy_res = copy_to_user ( dest_addr, &d, sizeof(d) );
    break;
  }
  default:
    break;
  }

  if (copy_res) {
    return -EFAULT;
  } else {
    return 0;
  }
}


/* Write a small number of bytes taken from user space */
ssize_t aclpci_write_small (void *write_addr, void __user* src_addr, ssize_t len) {

  ssize_t copy_res = 0;
  switch (len) {
  case 1: {
    u8 d;
    copy_res = copy_from_user ( &d, src_addr, sizeof(d) );
    writeb ( d, write_addr );
    break;
  }
  case 2: {
    u16 d;
    copy_res = copy_from_user ( &d, src_addr, sizeof(d) );
    writew ( d, write_addr );
    break;
  }
  case 4: {
    u32 d;
    copy_res = copy_from_user ( &d, src_addr, sizeof(d) );
    writel ( d, write_addr );
    break;
  }
  case 8: {
    u64 d;
    copy_res = copy_from_user ( &d, src_addr, sizeof(d) );
    writeq ( d, write_addr );
    break;
  }
  default:
    break;
  }

  if (copy_res) {
    return -EFAULT;
  } else {
    return 0;
  }
}



/* Read or Write arbitrary length sequency starting at read_addr and put it into
 * user space at dest_addr. if 'reading' is set to 1, doing the read. If 0, doing
 * the write. */
static ssize_t aclpci_rw_large (void *dev_addr, void __user* user_addr,
                                  ssize_t len, char *buffer, int reading) {
  size_t bytes_left = len;
  size_t i, num_missed;
  u64 *ibuffer = (u64*)buffer;
  char *cbuffer;
  size_t offset, num_to_read;
  size_t chunk = BUF_SIZE;
  
  u64 startj, ej;
  u64 sj = 0, acc_readj = 0, acc_transfj = 0;
  
  startj = get_jiffies_64();
  
  /* Reading upto BUF_SIZE values, one int at a time, and then transfer
   * the buffer at once to user space. Repeat as necessary. */
  while (bytes_left > 0) {
    if (bytes_left < BUF_SIZE) {
      chunk = bytes_left;
    } else {
      chunk = BUF_SIZE;
    }
    
    if (!reading) {
      sj = get_jiffies_64();
      if (copy_from_user (ibuffer, user_addr, chunk)) {
        return -EFAULT;
      }
      acc_transfj += get_jiffies_64() - sj;
    }
    
    /* Read one u64 at a time until fill the buffer. Then copy the whole
     * buffer at once to user space. */
    sj = get_jiffies_64();
    num_to_read = chunk / sizeof(u64);
    for (i = 0; i < num_to_read; i++) {
      if (reading) {
        ibuffer[i] = readq ( ((u64*)dev_addr) + i);
      } else {
        writeq ( ibuffer[i], ((u64*)dev_addr) + i );
      }
    }
    
    /* If length is not a multiple of sizeof(u64), will miss last few bytes.
     * In that case, read it one byte at a time. This can only happen on 
     * last iteration of the while() loop. */
    offset = num_to_read * sizeof(u64);
    num_missed = chunk - offset;
    cbuffer = (char*)(ibuffer + num_to_read);
    
    for (i = 0; i < num_missed; i++) {
      if (reading) {
        cbuffer[i] = readb ( (u8*)(dev_addr) + offset + i );
      } else {
        writeb ( cbuffer[i], (u8*)(dev_addr) + offset + i );
      }
    }
    acc_readj += get_jiffies_64() - sj;
    
    if (reading) {
      sj = get_jiffies_64();
      if (copy_to_user (user_addr, ibuffer, chunk)) {
        return -EFAULT;
      }
      acc_transfj += get_jiffies_64() - sj;
    }
    
    dev_addr += chunk;
    user_addr += chunk;
    bytes_left -= chunk;
  }
  
  ej = get_jiffies_64();
  ACL_VERBOSE_DEBUG (KERN_DEBUG "Spent %u msec %sing %lu bytes", jiffies_to_msecs(ej - startj), 
                          reading ? "read" : "writ", len);
  ACL_VERBOSE_DEBUG (KERN_DEBUG "  Dev access %u msec. User space transfer %u msec",
                        jiffies_to_msecs(acc_readj),
                        jiffies_to_msecs(acc_transfj));
  return 0;
}

/* Set CRA window so raw_user_ptr is "visible" to the BAR.
 * Return pointer to use to access the user memory */
static void* aclpci_set_segment (struct aclpci_dev *aclpci, void * raw_user_ptr) {

  ssize_t cur_segment = ((ssize_t)raw_user_ptr) / ACL_PCIE_MEMWINDOW_SIZE;  
  aclpci_set_segment_by_val (aclpci, cur_segment);  

  /* Can use the return value in all read/write functions in this file now */
  return (void*)((ssize_t)raw_user_ptr % ACL_PCIE_MEMWINDOW_SIZE);
}


/* Both start and end, user and device addresses must be 
 * 64-byte aligned to use DMA */
int aligned_request (struct aclpci_cmd *cmd, size_t count) {
  
  return (( (unsigned long)cmd->user_addr   & DMA_ALIGNMENT_BYTE_MASK) | 
          ( (unsigned long)cmd->device_addr & DMA_ALIGNMENT_BYTE_MASK) |
          ( count                           & DMA_ALIGNMENT_BYTE_MASK)
         ) == 0;
}                           


/* High-level read/write dispatcher. */
ssize_t aclpci_rw(struct file *file, char __user *buf, 
                  size_t count, loff_t *pos,
                  int reading) {
  
  struct aclpci_dev *aclpci = (struct aclpci_dev *)file->private_data;
  struct aclpci_cmd __user *ucmd;
  struct aclpci_cmd kcmd;
  u64 old_segment = 0;
  int restore_segment = 0;
  void *addr = 0;
  int aligned = 0;
  int use_dma = 0;
  ssize_t result = 0;
  ssize_t errno = 0;
  
  if (down_interruptible(&aclpci->sem)) {
    return -ERESTARTSYS;
  }
  
  ucmd = (struct aclpci_cmd __user *) buf;
  if (copy_from_user (&kcmd, ucmd, sizeof(*ucmd))) {
		result = -EFAULT;
    goto done;
	}
  
  if (kcmd.bar_id == ACLPCI_CMD_BAR) {
    /* This is not a read but a special command. */
    result = aclpci_exec_cmd (aclpci, kcmd, count);
    goto done;
  }
  
  /* Only using DMA for large aligned reads/writes on global memory
   * (due to some assumptions inside the DMA code). */
  aligned = aligned_request (&kcmd, count);
  use_dma = USE_DMA && (count >= 1024) && 
            aligned && kcmd.bar_id == ACL_PCI_GLOBAL_MEM_BAR;
  
  ACL_VERBOSE_DEBUG (KERN_DEBUG "\n\n-----------------------");
  ACL_VERBOSE_DEBUG (KERN_DEBUG " kcmd = {%u, %p, %p}, count = %lu", 
             kcmd.bar_id, (void*)kcmd.device_addr, (void*)kcmd.user_addr, count);
  if (!use_dma) {
    addr = aclpci_get_checked_addr (kcmd.bar_id, kcmd.device_addr, count, aclpci, &errno, 0);
    /* If not using DMA, need 'addr', which is mem-mapped virtual address. If that's out of
     * range of the BAR, can't do the operation. For DMA, mem-mapped virtual addresses are not
     * used. So don't care if they're in the BAR range or not. */
    if (errno != 0) {
    
      /* For global memory accesses, assume that the caller just forgot to call
       * set_segment. So do it for him and keep going. */
      if (kcmd.bar_id == ACL_PCI_GLOBAL_MEM_BAR) {
        ACL_VERBOSE_DEBUG (KERN_DEBUG "For global memory accesses, trying to change segment so the address is mapped into PCIe BAR");
        old_segment = aclpci->global_mem_segment;
        restore_segment = 1;
        kcmd.device_addr = aclpci_set_segment (aclpci, kcmd.device_addr);
        addr = aclpci_get_checked_addr (kcmd.bar_id, kcmd.device_addr, count, aclpci, &errno, 1);
      }
      
      if (errno != 0) {
        result = -EFAULT;
        goto done;
      }
    }
  }


  /* Intercept global mem segment changes to keep internal structures up-to-date */
  if (kcmd.bar_id == ACL_PCIE_MEMWINDOW_BAR) {
    if (addr == aclpci->global_mem_segment_addr) {
      u64 d;
      ACL_VERBOSE_DEBUG (KERN_DEBUG "Intercepted mem segment change to %llu", d);
      if (copy_from_user ( &d, kcmd.user_addr, sizeof(d) )) {
        result = -EFAULT;
        goto done;
      }
      aclpci->global_mem_segment = d;
    }
  }
  
  
  /* Offset value is always an address offset, not element offset. */
  /* ACL_DEBUG (KERN_DEBUG "Read address is %p", addr); */
  
  switch (count) {
  case 1:
  case 2:
  case 4:
  case 8: {
    if (reading) {
      result = aclpci_read_small (addr, (void __user*) kcmd.user_addr, count);
    } else {
      result = aclpci_write_small (addr, (void __user*) kcmd.user_addr, count);
    }
    break;
  }
    
  default:
    if (use_dma) {
      result = aclpci_dma_rw (aclpci, kcmd.device_addr, (void __user*) kcmd.user_addr, count, reading);
    } else {
      result = aclpci_rw_large (addr, (void __user*) kcmd.user_addr, count, aclpci->buffer, reading);
    }
    break;
  }
  
  /* If had to change the segment to get this read through, restore the value */
  if (restore_segment) {
    ACL_VERBOSE_DEBUG (KERN_DEBUG "Restoring mem segment to %llu", old_segment);
    aclpci_set_segment_by_val (aclpci, old_segment);
  }
  
done:
  up (&aclpci->sem);
  return result;
}


/* Response to user's read() call */
ssize_t aclpci_read(struct file *file, char __user *buf, 
                    size_t count, loff_t *pos) {
  
  return aclpci_rw (file, buf, count, pos, 1 /* reading */);
}


/* Response to user's write() call */
ssize_t aclpci_write(struct file *file, const char __user *buf, 
                     size_t count, loff_t *pos) {
                     
  return aclpci_rw (file, (char __user *)buf, count, pos, 0 /* writing */);
}

