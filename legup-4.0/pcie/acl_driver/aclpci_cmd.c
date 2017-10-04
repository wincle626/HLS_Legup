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


/* Handling of special commands (anything that is not read/write/open/close)
 * that user may call.
 * See pcie_linux_driver_exports.h for explanations of each command. */


#include <linux/mm.h>
#include <linux/device.h>
#include <linux/sched.h>
#include <linux/aer.h>

#include "aclpci.h"

/* RedHat 5.5 doesn't define this */
#ifndef PCI_EXP_LNKSTA_NLW_SHIFT
#define PCI_EXP_LNKSTA_NLW_SHIFT 4
#endif


void retrain_gen2 (struct aclpci_dev *aclpci);
void disable_aer_on_upstream_dev(struct aclpci_dev *aclpci);
void restore_aer_on_upstream_dev(struct aclpci_dev *aclpci);


/* Execute special command */
ssize_t aclpci_exec_cmd (struct aclpci_dev *aclpci, 
                         struct aclpci_cmd kcmd, 
                         size_t count) {
  ssize_t result = 0;

  switch (kcmd.command) {
  case ACLPCI_CMD_SAVE_PCI_CONTROL_REGS: {
    /* Disable interrupts before reprogramming. O/w the board will get into
     * a funny state and hang the system . */
    ACL_DEBUG (KERN_DEBUG "Saving PCI control registers");
    disable_aer_on_upstream_dev(aclpci);
    release_irq (aclpci->pci_dev, aclpci);    
    result = pci_save_state(aclpci->pci_dev);
    break;
  }

  case ACLPCI_CMD_LOAD_PCI_CONTROL_REGS: {
    
    pci_set_master(aclpci->pci_dev);
    init_irq (aclpci->pci_dev, aclpci);
    pci_restore_state(aclpci->pci_dev);
    restore_aer_on_upstream_dev(aclpci);
    retrain_gen2(aclpci);
    ACL_DEBUG (KERN_DEBUG "Restored PCI control registers");
    break;
  }

  case ACLPCI_CMD_PIN_USER_ADDR:
//    result = aclpci_pin_user_addr (kcmd.user_addr, count);
    break;

  case ACLPCI_CMD_UNPIN_USER_ADDR:
//    result = aclpci_unpin_user_addr (kcmd.user_addr, count);
    break;
    
  case ACLPCI_CMD_GET_DMA_IDLE_STATUS: {
    u32 idle = aclpci_dma_get_idle_status(aclpci);
    result = copy_to_user ( kcmd.user_addr, &idle, sizeof(idle) );
    break;
  }

  case ACLPCI_CMD_DMA_UPDATE: {
    aclpci_dma_update(aclpci, 0);
    break;
  }

  case ACLPCI_CMD_GET_DEVICE_ID: {
    u32 id = aclpci->pci_dev->device;
    result = copy_to_user ( kcmd.user_addr, &id, sizeof(id) );
    break;
  }
  
  case ACLPCI_CMD_GET_VENDOR_ID: {
    u32 id = aclpci->pci_dev->vendor;
    result = copy_to_user ( kcmd.user_addr, &id, sizeof(id) );
    break;
  }
 
  case ACLPCI_CMD_GET_PCI_GEN: {
    u32 pci_gen = aclpci->pci_gen;
    result = copy_to_user ( kcmd.user_addr, &pci_gen, sizeof(pci_gen) );
    break;
  }

  case ACLPCI_CMD_GET_PCI_NUM_LANES: {
    u32 pci_num_lanes = aclpci->pci_num_lanes;
    result = copy_to_user ( kcmd.user_addr, &pci_num_lanes, sizeof(pci_num_lanes) );
    break;
  }

 
  case ACLPCI_CMD_DO_CVP: {
    result = aclpci_cvp (aclpci, kcmd.user_addr, count);
    if (result != 0) {
      ACL_DEBUG (KERN_DEBUG "CvP failed.");
    }
    break;
  }

  case ACLPCI_CMD_SET_SIGNAL_PAYLOAD: {
    u32 id;
    result = copy_from_user ( &id, kcmd.user_addr, sizeof(id) );
    aclpci->signal_info.si_int = id;
    break;
  }
  
  case ACLPCI_CMD_GET_DRIVER_VERSION: {
    /* Driver version is a string */
    result = copy_to_user ( kcmd.user_addr, &ACL_DRIVER_VERSION, strlen(ACL_DRIVER_VERSION)+1 );
    break;
  }

  default:
    ACL_DEBUG (KERN_WARNING " Invalid command id %u! Ignoring the call. See aclpci_common.h for list of understood commands", kcmd.command);
    result = -EFAULT;
    break;
  }
  
  return result;
}




/* Pinning user pages.
 * 
 * Taken from <kernel code>/drivers/infiniband/hw/ipath/ipath_user_pages.c
 */
static void __aclpci_release_user_pages(struct page **p, size_t num_pages,
				   int dirty)
{
	size_t i;

	for (i = 0; i < num_pages; i++) {
		if (dirty) {
			set_page_dirty_lock(p[i]);
      }
		put_page(p[i]);
	}
}

/* call with current->mm->mmap_sem held */
static int __aclpci_get_user_pages(unsigned long start_page, size_t num_pages,
			struct page **p, struct vm_area_struct **vma)
{
	size_t got;
	int ret;

	for (got = 0; got < num_pages; got += ret) {
		ret = get_user_pages(current, current->mm,
				     start_page + got * PAGE_SIZE,
				     num_pages - got, 1, 1,
				     p + got, vma);
		if (ret < 0)
			goto bail_release;
	}

	current->mm->locked_vm += num_pages;

	ret = 0;
	goto bail;

bail_release:
	__aclpci_release_user_pages(p, got, 0);
bail:
	return ret;
}


/**
 * aclpci_get_user_pages - lock user pages into memory
 * @start_page: the start page
 * @num_pages: the number of pages
 * @p: the output page structures
 *
 * This function takes a given start page (page aligned user virtual
 * address) and pins it and the following specified number of pages.
 */
int aclpci_get_user_pages(unsigned long start_page, size_t num_pages,
			 struct page **p)
{
	int ret;

	down_write(&current->mm->mmap_sem);
	ret = __aclpci_get_user_pages(start_page, num_pages, p, NULL);
	up_write(&current->mm->mmap_sem);

	return ret;
}

void aclpci_release_user_pages(struct page **p, size_t num_pages)
{
	down_write(&current->mm->mmap_sem);

	__aclpci_release_user_pages(p, num_pages, 1);

	current->mm->locked_vm -= num_pages;

	up_write(&current->mm->mmap_sem);
}


/* Check link speed and retrain it to gen2 speeds.
 * After reprogramming, the link defaults to gen1 speeds for some reason.
 * Doing re-training by finding the upstream root device and telling it
 * to retrain itself. Doesn't seem to be a cleaner way to do this. */
void retrain_gen2 (struct aclpci_dev *aclpci) {

  struct pci_dev *dev = aclpci->pci_dev;
  u16 linkstat, speed, width;
  struct pci_dev *upstream;
  int pos, upos;
  u16 status_reg, control_reg, link_cap_reg;
  u16 status, control;
  u32 link_cap;
  int training, timeout;
  
  /* Defines for some special PCIe control bits */
  #define DISABLE_LINK_BIT         (1 << 4)
  #define RETRAIN_LINK_BIT         (1 << 5)
  #define TRAINING_IN_PROGRESS_BIT (1 << 11)
  #define LINKSPEED_2_5_GB         (0x1)
  #define LINKSPEED_5_0_GB         (0x2)
   
  pos = pci_find_capability (dev, PCI_CAP_ID_EXP);
  if (!pos) {
    ACL_DEBUG (KERN_WARNING "Can't find PCI Express capability!");
    return;
  }

  /* Find root node for this bus and tell it to retrain itself. */
  upstream = aclpci->upstream;
  if (upstream == NULL) {
    return;
  }
  upos = pci_find_capability (upstream, PCI_CAP_ID_EXP);
  status_reg = upos + PCI_EXP_LNKSTA;
  control_reg = upos + PCI_EXP_LNKCTL;
  link_cap_reg = upos + PCI_EXP_LNKCAP;
  pci_read_config_word (upstream, status_reg, &status);
  pci_read_config_word (upstream, control_reg, &control);
  pci_read_config_dword (upstream, link_cap_reg, &link_cap);
  
  
  pci_read_config_word (dev, pos + PCI_EXP_LNKSTA, &linkstat);
  pci_read_config_dword (upstream, link_cap_reg, &link_cap);
  speed = linkstat & PCI_EXP_LNKSTA_CLS;
  width = (linkstat & PCI_EXP_LNKSTA_NLW) >> PCI_EXP_LNKSTA_NLW_SHIFT;
  
  aclpci->pci_gen = (speed == LINKSPEED_5_0_GB) ? 2 : 1;
  aclpci->pci_num_lanes = width;
      
  if (speed == LINKSPEED_2_5_GB) {
    ACL_DEBUG (KERN_DEBUG "Link is operating at 2.5 GT/s with %d lanes. Need to retrain.", width);
  } else if (speed == LINKSPEED_5_0_GB) {
    ACL_DEBUG (KERN_DEBUG "Link is operating at 5.0 GT/s with %d lanes.", width);
    if (width == 4) {
      ACL_DEBUG (KERN_DEBUG "  All is good!");
      return;
    } else {
      ACL_DEBUG (KERN_DEBUG "  Need to retrain.");
    }
  } else {
    ACL_DEBUG (KERN_WARNING "Not sure what's going on. Retraining.");
  }
  
      
  /* Perform the training. */
  training = 1;
  timeout = 0;
  pci_read_config_word (upstream, control_reg, &control);
  pci_write_config_word (upstream, control_reg, control | RETRAIN_LINK_BIT);
  
  while (training && timeout < 50)
  {
    pci_read_config_word (upstream, status_reg, &status);
    training = (status & TRAINING_IN_PROGRESS_BIT);
    msleep (1); /* 1 ms */
    ++timeout;
  }
  if(training)
  {
     ACL_DEBUG (KERN_DEBUG "Error: Link training timed out.");
     ACL_DEBUG (KERN_DEBUG "PCIe link not established.");
  }
  else
  {
     ACL_DEBUG (KERN_DEBUG "Link training completed in %d ms.", timeout);
  }
   

  /* Verify that it's a 5 GT/s link now */
  pci_read_config_word (dev, pos + PCI_EXP_LNKSTA, &linkstat);
  pci_read_config_dword (upstream, link_cap_reg, &link_cap);
  speed = linkstat & PCI_EXP_LNKSTA_CLS;
  width = (linkstat & PCI_EXP_LNKSTA_NLW) >> PCI_EXP_LNKSTA_NLW_SHIFT;
  
  aclpci->pci_gen = (speed == LINKSPEED_5_0_GB) ? 2 : 1;
  aclpci->pci_num_lanes = width;
  
  if(speed == LINKSPEED_5_0_GB)
  {
    ACL_DEBUG (KERN_DEBUG "Link operating at 5 GT/s with %d lanes", width);
  }
  else
  {
    ACL_DEBUG (KERN_WARNING "** WARNING: Link training failed.  Link operating at 2.5 GT/s with %d lanes.\n", width);
  }
  
  return;
}


/* For some reason, pci_find_ext_capability is not resolved
 * when loading this driver. So copied the implementation here. */
#define PCI_CFG_SPACE_SIZE 256
#define PCI_CFG_SPACE_EXP_SIZE 4096
 int my_pci_find_ext_capability(struct pci_dev *dev, int cap)
{
  u32 header;
  int ttl;
  int pos = PCI_CFG_SPACE_SIZE;

  /* minimum 8 bytes per capability */
  ttl = (PCI_CFG_SPACE_EXP_SIZE - PCI_CFG_SPACE_SIZE) / 8;

  if (dev->cfg_size <= PCI_CFG_SPACE_SIZE)
    return 0;

  if (pci_read_config_dword(dev, pos, &header) != PCIBIOS_SUCCESSFUL)
    return 0;

  /*
  * If we have no capabilities, this is indicated by cap ID,
  * cap version and next pointer all being 0.
  */
  if (header == 0)
    return 0;

  while (ttl-- > 0) {
  if (PCI_EXT_CAP_ID(header) == cap)
    return pos;

  pos = PCI_EXT_CAP_NEXT(header);
  if (pos < PCI_CFG_SPACE_SIZE)
    break;

  if (pci_read_config_dword(dev, pos, &header) != PCIBIOS_SUCCESSFUL)
    break;
  }

  return 0;
}


/* return value of AER uncorrectable error mask register
 * for UPSTREAM node of the given device. */
u32 get_aer_uerr_mask_reg (struct aclpci_dev *aclpci)
{
  struct pci_dev *dev = aclpci->upstream;
  u32 reg32 = 0;
  int pos;

  if (dev == NULL) {
    ACL_DEBUG (KERN_DEBUG "No upstream device found!");
    return -EIO;
  }
  
  /*if (dev->aer_firmware_first) {
    return -EIO;
  }*/

  pos = my_pci_find_ext_capability(dev, PCI_EXT_CAP_ID_ERR);
  if (!pos) {
    ACL_DEBUG (KERN_DEBUG "Upstream device doesn't have AER extended capability.");
    return -EIO;
  }
  
  pci_read_config_dword(dev, pos+0x4, &reg32);
  pci_read_config_dword(dev, pos+0x8, &reg32);
  return reg32;
}

/* Surprise down is the 5th register inside AER uncorrectable error register/mask */
#define AER_SURPRISE_DOWN 0x20


/* Set AER uncorrectable error mask register
 * for UPSTREAM node of the given device. */
void set_aer_uerr_mask_reg (struct aclpci_dev *aclpci, u32 val)
{
  int pos;
  struct pci_dev *dev = aclpci->upstream;
  if (!dev) {
    return;
  }
    
  pos = my_pci_find_ext_capability(dev, PCI_EXT_CAP_ID_ERR);
  if (!pos) {
    return;
  }

  /* First, clear the error bit by writing 1 to 5th bit. */
  pci_write_config_dword(dev, pos+0x4, AER_SURPRISE_DOWN);

  /* Now set the mask register */
  pci_write_config_dword(dev, pos+0x8, val);
  return;
}


/* Mask off "Surprise Down" error in AER. Note that setting the mask to '1' means
 * the error is ignored. */
void disable_aer_on_upstream_dev(struct aclpci_dev *aclpci) {

  u32 disabled;
  aclpci->aer_uerr_mask_reg = get_aer_uerr_mask_reg(aclpci);
  if (aclpci->aer_uerr_mask_reg == -EIO) {
    return;
  }
    
  disabled = aclpci->aer_uerr_mask_reg | AER_SURPRISE_DOWN;
  
  ACL_DEBUG (KERN_WARNING "Changing AER Uncorrectable error mask register from %x to %x", 
             aclpci->aer_uerr_mask_reg, disabled);
  set_aer_uerr_mask_reg(aclpci, disabled);
}


/* Restore AER uncorrectable error mask register
 * for UPSTREAM node of the given device. */
void restore_aer_on_upstream_dev(struct aclpci_dev *aclpci) {

  if (aclpci->aer_uerr_mask_reg == -EIO)
    return;
  ACL_DEBUG (KERN_WARNING "Restoring AER Uncorrectable error mask register to %x", aclpci->aer_uerr_mask_reg);
  set_aer_uerr_mask_reg(aclpci, aclpci->aer_uerr_mask_reg);
}
