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

/* DMA logic imlementation.
 *
 * The basic flow of DMA transfer is as follows:
 *  1. Pin user memory (a contiguous set of address in processor address space)
 *     to get a list of physical pages (almost never contiguous list of 4KB
 *     blocks).
 *  2. Setup ATT (address translation table) entries for physical page address 
 *     to "row #" mapping in the FPGA PCIe core (a "row number" is just a
 *     counter that is passed in DMA descriptor as a host address).
 *  3. Create and send DMA descriptor (src addr, dest addr, and lengh) to the DMA
 *     core to perform the transfer. 
 *  4. Go to step 2 if have not transfered all currently pinned memory yet.
 *  5. Go to step 1 if need to pin more memory.
 *
 * DMA controller sends an interrupt whenever work for a single DMA descriptor
 * is complete. The driver also checks "done_count" to see how many descriptors
 * completed. This is in case the driver misses an interrupt.
 *
 * To keep interrupt logic simple (i.e. no work-queues), the user has to stall
 * using the following logic until DMA is done:
 *
 *   while (!dma->is_idle())
 *      dma->update(0);
 *
 * The DMA logic is complicated because there are a number of limits of what can
 * be done in one shot. Here they are (all are constants in hw_pcie_constants.h):
 *  - How much user memory can be pinned at one time.
 *  - Size of ATT table in hardware
 *  - Number of outstanding descriptors the hardware can have.
 *
 * Due to hardware restrictions, can only do DMA for 32-byte aligned start
 * addresses (on both host and device) AND 32-byte aligned lengths.
 * Also, need to use a separate descriptor for transfers that do NOT start or
 * end on page boundary. DMA engine does NOT do DMA for these cases, so these
 * transfers are very slow. */


#include <linux/mm.h>
#include <linux/scatterlist.h>
#include <linux/sched.h>
#include <asm/page.h>
#include <linux/spinlock.h>
#include <linux/jiffies.h>

#include "aclpci.h"


#include <linux/mm.h>


#if USE_DMA


#ifdef ACLPCI_BIGENDIAN

/* Swap bytes x[A] and x[B] */
#define SWAP(x,A,B) FLIP_TEMP = ((char*)(x))[A]; ((char*)(x))[A] = ((char*)(x))[B]; ((char*)(x))[B] = FLIP_TEMP;

/* Flip 4-byte ints inside 8-byte location to deal with endianness
 * Equivalent to flip8 followed by two flip4 */
#define FLIP4_8(x) \
{ char SWAP(x,0,4) \
       SWAP(x,1,5) \
       SWAP(x,2,6) \
       SWAP(x,3,7) \
       SWAP(x,4,0) \
       SWAP(x,5,1) \
       SWAP(x,6,2) \
       SWAP(x,7,3) }

#else // ACLPCI_BIGENDIAN

#define SWAP(x,A,B)
#define FLIP4_8(x)

#endif


/* Map/Unmap pages for DMA transfer.
 * All docs say I need to do it but unmapping pages after
 * reading clears their content. */
#define MAP_UNMAP_PAGES 0
#define DEBUG_UNLOCK_PAGES MAP_UNMAP_PAGES


/* Forward declarations */
int read_write (struct aclpci_dev* aclpci, void* src, void *dst, size_t bytes, int reading);
void set_att_entry (struct aclpci_dev* aclpci, unsigned long address, unsigned int row);
void send_active_descriptor (struct aclpci_dev* aclpci);

unsigned long acl_get_dma_offset (void) {
   return ACL_PCIE_DMA_OFFSET - ACL_PCIE_MEMWINDOW_SIZE;
}
unsigned long get_dma_desc_offset(void) {
   return ACL_PCIE_DMA_DESCRIPTOR_OFFSET - ACL_PCIE_MEMWINDOW_SIZE;
}

static unsigned long CRA_OFFSET      = 0;



/* Get memory-mapped address on the device. 
 * Assuming asking for DMA control region, so have hard-coded BAR value. */
void *get_dev_addr(struct aclpci_dev *aclpci, void *addr, ssize_t data_size) {

  ssize_t errno = 0;
  void *dev_addr = 0;
  struct aclpci_dma *d = &(aclpci->dma_data);
  
  if (d->m_aclpci == NULL) return dev_addr;
  
  dev_addr = aclpci_get_checked_addr (ACL_PCIE_DMA_BAR, addr, data_size, d->m_aclpci, &errno, 1);
  if (errno != 0) {
    ACL_DEBUG (KERN_DEBUG "ERROR: addr failed check");
    return NULL;
  }
  return dev_addr;
}


/* write 32 bits to DMA control region */
void dma_write32(struct aclpci_dev *aclpci, ssize_t addr, u32 data) {

  unsigned long dma_offset = acl_get_dma_offset();

  void *dev_addr = get_dev_addr (aclpci, (void*)addr + dma_offset, sizeof(u32));
  ACL_VERBOSE_DEBUG (KERN_DEBUG "DMA: Writing 32 bytes %u, addr = 0x%p, dev_addr = 0x%p, dma_offset = 0x%lx",
                     data, addr , dev_addr, dma_offset);
  if (dev_addr == NULL) return;
  writel (data, dev_addr);
  
  ACL_VERBOSE_DEBUG (KERN_DEBUG "   DMA: Read back 32 bytes (%u) from 0x%p", readl (dev_addr), dev_addr);
}

/* read 32 bits from DMA control region */
ssize_t dma_read32(struct aclpci_dev *aclpci, ssize_t addr) {
  unsigned long dma_offset = acl_get_dma_offset();
  void *dev_addr = get_dev_addr (aclpci, (void*)addr + dma_offset, sizeof(u32));
  ACL_VERBOSE_DEBUG (KERN_DEBUG "DMA: Reading 32 bytes from 0x%p", addr);
  return readl (dev_addr);
}

union u64_2 {
  struct DMA_DESCRIPTOR desc;
  u64 ints[2];
};


/* Write a single DMA descriptor to the DMA control region. */
void dma_desc_write(struct aclpci_dev *aclpci, void *addr, struct DMA_DESCRIPTOR *desc) {
  unsigned long dma_desc_offset = get_dma_desc_offset();

  size_t u64_size = sizeof(u64);
  union u64_2 u;
  void *dev_addr = get_dev_addr (aclpci, addr + dma_desc_offset, sizeof(u64));
  if (u64_size * 2 != sizeof (struct DMA_DESCRIPTOR)) {
    ACL_DEBUG (KERN_WARNING "Size of DMA_DESCRIPTOR is not 2 x sizeof(u64)");
    return;
  }
  
  ACL_VERBOSE_DEBUG (KERN_DEBUG "DMA_DESC: Writing %lu bytes to 0x%p (dev_addr = 0x%p, 0x%lx)", 
              sizeof (struct DMA_DESCRIPTOR), 
              addr, dev_addr, dma_desc_offset);
  if (dev_addr == NULL) return;
  
  u.desc = *desc;
  /* Take care of endianness. writeq will flip the data as if they were 8-byte ints
   * but transferring 4-byte ints instead. */
  FLIP4_8(&u.ints[0]); writeq (u.ints[0], dev_addr);
  FLIP4_8(&u.ints[1]); writeq (u.ints[1], dev_addr + u64_size);
}


u32 pcie_cra_read32(struct aclpci_dev *aclpci, void *addr) {
  
  void *dev_addr = get_dev_addr (aclpci, addr + CRA_OFFSET, sizeof(u32));
  ACL_VERBOSE_DEBUG (KERN_DEBUG "PCIE_CRA: Reading 32 bytes from 0x%p", dev_addr);
  return readl (dev_addr);
}

void pcie_cra_write32(struct aclpci_dev *aclpci, void *addr, u32 data) {
  
  void *dev_addr = get_dev_addr (aclpci, addr + CRA_OFFSET, sizeof(u32));
  ACL_VERBOSE_DEBUG (KERN_DEBUG "PCIE_CRA: Writing 32 bytes (%u) to 0x%p", data, addr);
  writel (data, dev_addr); 
  
  ACL_VERBOSE_DEBUG (KERN_DEBUG "   PCIE_CRA: read back 32 bytes (%u) to 0x%p", readl(dev_addr), addr);
}

void pcie_cra_write64(struct aclpci_dev *aclpci, void *addr, u64 data) {
  
  void *dev_addr = get_dev_addr (aclpci, addr + CRA_OFFSET, sizeof(u64));
  ACL_VERBOSE_DEBUG (KERN_DEBUG "PCIE_CRA: Writing 64 bytes (%llu) to 0x%p", data, dev_addr);
  writeq (data, dev_addr);
  
  ACL_VERBOSE_DEBUG (KERN_DEBUG "   PCIE_CRA: read back 64 bytes (%llu) to 0x%p", readq(dev_addr), dev_addr);
}



int is_idle (struct aclpci_dev *aclpci) {
  struct aclpci_dma *d = &(aclpci->dma_data);
  return d->m_idle;
}

// We can only pin a limited amount of memory
int pending_dma_full (struct aclpci_dev *aclpci) {
  struct aclpci_dma *d = &(aclpci->dma_data);
  return queue_size(&d->m_dma_pending) >= ACL_PCIE_DMA_MAX_PINNED_MEM;
}

// Checks our representation of the hardware FIFOs to verify they are not full
int hardware_buffers_full (struct aclpci_dev *aclpci) { 
  struct aclpci_dma *d = &(aclpci->dma_data);
  return  (queue_size(&d->m_desc_pending) >= ACL_PCIE_DMA_MAX_DESCRIPTORS) 
           || (d->m_att_size >= ACL_PCIE_DMA_MAX_ATT_SIZE);
}

// If there is an active transfer, check that the hardware is free to accept
// more data; otherwise, check that there is more data to send and we are
// free to lock a new buffer
int ready_for_data (struct aclpci_dev *aclpci) {
  struct aclpci_dma *d = &(aclpci->dma_data);
  if (d->m_active_mem.dma.ptr == NULL) {
    return (d->m_bytes_sent < d->m_bytes) && !pending_dma_full(aclpci);
  } else {
    return !hardware_buffers_full(aclpci);
  }
}

/* Add a byte-offset to a void* pointer */
void* compute_address (void* base, unsigned int offset)
{
   unsigned long p = (unsigned long)(base);
   return (void*)(p + offset);
}


/* Init DMA engine. Should be done at device load time */
void aclpci_dma_init(struct aclpci_dev *aclpci) {

  struct aclpci_dma *d = &(aclpci->dma_data);
  d->m_att_size = 0;
  d->m_next_att_row = 0;
  memset( &d->m_active_mem, 0, sizeof(struct pinned_mem) );
  d->m_active_descriptor_valid = false;
  d->m_old_done_count = 0;
  d->m_idle=1;
  
  d->m_aclpci = aclpci;
  d->m_pci_dev = aclpci->pci_dev;

  /* Enable DMA controller */
  dma_write32 (aclpci, DMA_CSR_CONTROL, ACL_PCIE_GET_BIT(DMA_CTRL_IRQ_ENABLE));
  
  // Resets the "done-count" register
  dma_write32 (aclpci, DMA_CSR_STATUS, ACL_PCIE_GET_BIT(DMA_STATUS_COUNT_LO));

  d->m_descriptors_updated = 0;
  d->m_descriptors_acknowledged = 0;

  queue_init (&d->m_dma_pending, sizeof(struct dma_t), ACL_PCIE_DMA_MAX_PINNED_MEM);
  queue_init (&d->m_desc_pending, sizeof(struct DESCRIPTOR_UPDATE_DATA), ACL_PCIE_DMA_MAX_DESCRIPTORS);
}


void aclpci_dma_finish(struct aclpci_dev *aclpci) {

  struct aclpci_dma *d = &(aclpci->dma_data);
  queue_fini (&d->m_dma_pending);
  queue_fini (&d->m_desc_pending);
  
  /* Disable DMA controller */
  dma_write32 (aclpci, DMA_CSR_CONTROL, 0);
}


/* Called by main interrupt handler in aclpci.c. By the time we get here,
 * we know it's a DMA interrupt. So only need to do DMA-related stuff. */
irqreturn_t aclpci_dma_service_interrupt (struct aclpci_dev *aclpci)
{
  unsigned int dma_status;
  unsigned int done_count;
  struct aclpci_dma *d = &(aclpci->dma_data);

  // Clear the IRQ bit
  dma_write32 (aclpci, DMA_CSR_STATUS, ACL_PCIE_GET_BIT(DMA_STATUS_IRQ) );

  // Read the DMA status register
  dma_status = dma_read32 (aclpci, DMA_CSR_STATUS );

  // Compute how many descriptors completed
  done_count = ACL_PCIE_READ_BIT_RANGE( dma_status, DMA_STATUS_COUNT_HI, DMA_STATUS_COUNT_LO );

  ACL_VERBOSE_DEBUG (KERN_DEBUG "dma_status = %x, done_count = %u", dma_status, done_count);
  
  /* The done_count counter can wrap around - but it's not possible for it to wrap more than
   * once in a single update.
   *
   * CONCURRENCY NOTE: m_descriptors_acknowledged is concurrently updated by
   *          this code AND by aclpci_dma_update(). However, no locks are
   *          required because of the way this variable is used.
   *                      Dmitry May 2012
   */
  if(done_count < d->m_old_done_count)
    d->m_descriptors_acknowledged += (done_count + ACL_PCIE_DMA_MAX_DONE_COUNT - d->m_old_done_count);
  else
    d->m_descriptors_acknowledged += (done_count - d->m_old_done_count);
  d->m_old_done_count = done_count;
  
  ACL_VERBOSE_DEBUG (KERN_DEBUG "m_descriptors_acknowledged = %d. Sent - acks = %d",
                     d->m_descriptors_acknowledged, 
                     d->m_descriptors_sent - d->m_descriptors_acknowledged);
  
  return IRQ_HANDLED;
}


/* Read/Write large amounts of data using DMA.
 *   dev_addr  -- address on device to read to/write from
 *   dest_addr -- address in user space to read to/write from
 *   len       -- number of bytes to transfer
 *   reading   -- 1 if doing read (from device), 0 if doing write (to device)
 */
ssize_t aclpci_dma_rw (struct aclpci_dev *aclpci,
                       void *dev_addr, void __user* user_addr, 
                       ssize_t len, int reading) {

  ACL_VERBOSE_DEBUG (KERN_DEBUG "DMA: %sing %lu bytes", reading ? "Read" : "Writ", len);
  if (reading) {
    read_write (aclpci, dev_addr,  user_addr, len, reading);
  } else {
    read_write (aclpci, user_addr,  dev_addr, len, reading);
  }
  
  return 0;
}


/* Return idle status of the DMA hardware. */
int aclpci_dma_get_idle_status(struct aclpci_dev *aclpci) {
  return aclpci->dma_data.m_idle;
}


int lock_dma_buffer (struct aclpci_dev *aclpci, void *addr, ssize_t len, struct pinned_mem *active_mem) {

  int ret;
  unsigned int num_act_pages;
  struct aclpci_dma *d = &(aclpci->dma_data);
  ssize_t start_page, end_page, num_pages;
  u64 ej, startj = get_jiffies_64();
  struct dma_t *dma = &(active_mem->dma);
  
  dma->ptr = addr;
  dma->len = len;
  dma->dir = d->m_read ? PCI_DMA_FROMDEVICE : PCI_DMA_TODEVICE;
  /* num_pages that [addr, addr+len] map to. */
  start_page = (ssize_t)addr >> PAGE_SHIFT;
  end_page = ((ssize_t)addr + len - 1) >> PAGE_SHIFT;
  num_pages = end_page - start_page + 1;
  
  dma->num_pages = num_pages;
  dma->pages = (struct page**)kzalloc ( sizeof(struct page*) * dma->num_pages, GFP_KERNEL );
  if (dma->pages == NULL) {
    ACL_DEBUG (KERN_DEBUG "Couldn't allocate array of %u ptrs!", dma->num_pages);
    return -EFAULT;
  }
  
  dma->dma_addrs = (dma_addr_t*)kzalloc ( sizeof(dma_addr_t) * dma->num_pages, GFP_KERNEL );
  if (dma->dma_addrs == NULL) {
    ACL_DEBUG (KERN_DEBUG "Couldn't allocate array of %u dma_addr_t's!", dma->num_pages);
    return -EFAULT;
  }
  ACL_VERBOSE_DEBUG (KERN_DEBUG "pages = [%p, %p), dma_addrs = [%p, %p)", 
                     dma->pages, dma->pages+num_pages, dma->dma_addrs, dma->dma_addrs+num_pages);
  
  /* pin user memory and get set of physical pages back in 'p' ptr. */
  ret = aclpci_get_user_pages((unsigned long)addr & PAGE_MASK, num_pages, dma->pages);
  if (ret != 0) {
    ACL_DEBUG (KERN_DEBUG "Couldn't pin all user pages. %d!\n", ret);
    return -EFAULT;
  }
  
  
  /* map pages for PCI access. */
  num_act_pages = 0;
  #if MAP_UNMAP_PAGES
  unsigned int i;
  for (i = 0; i < dma->num_pages; i++) {
    struct page *cur = dma->pages[i];
    ACL_DEBUG (KERN_DEBUG "p[%d] = 0x%p", i, cur);
    if (cur != NULL) {
      ACL_DEBUG (KERN_DEBUG "  phys_addr = 0x%llx", page_to_phys(cur));
      dma_addr_t phys = pci_map_page (m_pci_dev, cur, 0, PAGE_SIZE, dma->dir);
      if (phys == 0) {
        ACL_DEBUG (KERN_DEBUG "  Couldn't pci_map_page!");
        return -EFAULT;
      }
      dma->dma_addrs[i] = phys;
      num_act_pages++;
    }
  }
  #endif

  active_mem->pages_rem = dma->num_pages;
  active_mem->next_page = dma->pages;
  active_mem->first_page_offset = (unsigned long)addr & ACL_PCIE_DMA_ATT_PAGE_ADDR_MASK;
  active_mem->last_page_offset = (unsigned long)(addr + len) & ACL_PCIE_DMA_ATT_PAGE_ADDR_MASK;
  
  //ACL_DEBUG (KERN_DEBUG  "Content of first page (addr  = %p): %s", 
  //             page_to_phys(dma->pages[0]), (char*)phys_to_virt(page_to_phys(dma->pages[0])));
  ej = get_jiffies_64();
  
  ACL_VERBOSE_DEBUG (KERN_DEBUG  "DMA: Pinned %u bytes (%lu pages) at 0x%p in %u usec", 
      (unsigned int)len, num_pages, addr, jiffies_to_usecs(ej - startj));
  ACL_VERBOSE_DEBUG (KERN_DEBUG  "DMA: first page offset is %u, last page offset is %u", 
         active_mem->first_page_offset, active_mem->last_page_offset);
         
  d->m_pin_time += (ej - startj);
  d->m_lock_time += (ej - startj);
  return 0;
}


void unlock_dma_buffer (struct aclpci_dev *aclpci, struct dma_t *dma) {

  struct aclpci_dma *d = &(aclpci->dma_data);
  u64 ej, startj = get_jiffies_64();
  
  #if DEBUG_UNLOCK_PAGES
  char *s = (char*)phys_to_virt(page_to_phys(dma->pages[0]));
  
  ACL_DEBUG (KERN_DEBUG  "1. Content of first page (addr  = %p): %s", 
               page_to_phys(dma->pages[0]), s);
  #endif
  
  #if MAP_UNMAP_PAGES
  int i;
  /* Unmap pages to make the data available for CPU */      
  for (i = 0; i < dma->num_pages; i++) {
    struct page *cur = dma->pages[i];
    // ACL_DEBUG (KERN_DEBUG "p[%d] = %p", i, cur);
    if (cur != NULL) {
      dma_addr_t phys = dma->dma_addrs[i];
      pci_unmap_page (m_pci_dev, phys, PAGE_SIZE, dma->dir);
    }
  }
  #endif
  
  // TODO: If do map/unmap for reads, the data is 0 by now!!!!
  #if DEBUG_UNLOCK_PAGES
    ACL_DEBUG (KERN_DEBUG  "2. Content of first page: %s", s);
  #endif

  /* Unpin pages */
  aclpci_release_user_pages (dma->pages, dma->num_pages);
    
  /* TODO: try to re-use these buffers on future allocs */
  kfree (dma->pages);
  kfree (dma->dma_addrs);

  ej = get_jiffies_64();
  ACL_VERBOSE_DEBUG (KERN_DEBUG  "DMA: Unpinned %u pages in %u usec", 
                          dma->num_pages,
                          jiffies_to_usecs(ej - startj));
                          
  /* Reset all dma fields. */
  memset (dma, 0, sizeof(struct dma_t));
  
  d->m_pin_time += (ej - startj);
  d->m_unlock_time += (ej - startj);
}


// Return 1 if something was done. 0 otherwise.
int aclpci_dma_update (struct aclpci_dev *aclpci, int forced)
{
   u64 startj;
   struct aclpci_dma *d = &(aclpci->dma_data);
   
   /* CONCURRENCY NOTE: d->m_descriptors_acknowledged is concurrently updated by
    *          the interrupt handler AND by this function. However, no locks are
    *          required because of the way this variable is used.
    *                      Dmitry May 2012
    */
   if ((d->m_descriptors_updated == d->m_descriptors_acknowledged) && !forced)
   {
      return 0;
   }

   startj = get_jiffies_64();
   
   // Process any descriptors that have completed
   ACL_VERBOSE_DEBUG (KERN_DEBUG "Descriptors: Updated - %d, Acknowledged - %d (forced %d)",
              d->m_descriptors_updated, d->m_descriptors_acknowledged, forced);

   while(d->m_descriptors_updated < d->m_descriptors_acknowledged)
   {
      struct DESCRIPTOR_UPDATE_DATA *front_desc = NULL;
      assert (!queue_empty(&d->m_desc_pending));
      front_desc = (struct DESCRIPTOR_UPDATE_DATA *)queue_front(&d->m_desc_pending);
      d->m_bytes_acknowledged += front_desc->bytes;
      d->m_att_size -= front_desc->att_entries;
      ACL_VERBOSE_DEBUG (KERN_DEBUG "DMA: Descriptor (%d bytes) completed", front_desc->bytes);
      queue_pop (&d->m_desc_pending);
      ++d->m_descriptors_updated;
   }
   ACL_VERBOSE_DEBUG (KERN_DEBUG  "DMA: %d descriptors pending...", queue_size (&d->m_desc_pending));

   // Process any pinned memory that is no longer required
   while( (d->m_bytes_acknowledged > 0) && 
          !queue_empty (&d->m_dma_pending) && 
          (((struct dma_t*)queue_front(&d->m_dma_pending))->len <= d->m_bytes_acknowledged) )
   {
      struct dma_t *dma = (struct dma_t*)queue_front (&d->m_dma_pending);
      d->m_bytes_acknowledged -= dma->len;
      unlock_dma_buffer (aclpci, dma);
      queue_pop (&d->m_dma_pending);
   }
   ACL_VERBOSE_DEBUG (KERN_DEBUG  "DMA: %d dma buffers pending...", queue_size (&d->m_dma_pending));
   ACL_VERBOSE_DEBUG (KERN_DEBUG  "DMA: %d bytes pending...", (d->m_bytes - d->m_bytes_sent));

   // Check if the transaction is complete
   if( (d->m_bytes_sent == d->m_bytes) && queue_empty (&d->m_dma_pending) )
   {
      assert(queue_empty(&d->m_desc_pending));
      assert(d->m_active_mem.dma.ptr == NULL);
      assert(!d->m_active_descriptor_valid);
      assert(d->m_att_size == 0);
      d->m_idle = 1;
      
      d->m_update_time += (get_jiffies_64() - startj);
      ACL_VERBOSE_DEBUG (KERN_DEBUG "DMA: Transaction complete! Sent %u descriptors",
                 d->m_descriptors_updated);
      ACL_VERBOSE_DEBUG (KERN_DEBUG "Times (us): Total = %u, update = %u, pin = %u", 
                 jiffies_to_usecs(get_jiffies_64() - d->m_start_time),
                 jiffies_to_usecs(d->m_update_time),
                 jiffies_to_usecs(d->m_pin_time)
                 );
      ACL_VERBOSE_DEBUG (KERN_DEBUG "Times (us): lock = %u, unlock = %u",
                 jiffies_to_usecs(d->m_lock_time),
                 jiffies_to_usecs(d->m_unlock_time));
      return 1;
   }

   // Send as much data as the HW can accept
   while( ready_for_data(aclpci) )
   {
      if (d->m_active_mem.dma.ptr == NULL)
      {
         unsigned int bytes_rem = (d->m_bytes - d->m_bytes_sent);
         unsigned int lock_size = (bytes_rem > ACL_PCIE_DMA_MAX_PINNED_MEM_SIZE) ? 
                             ACL_PCIE_DMA_MAX_PINNED_MEM_SIZE : 
                             bytes_rem;
         void* lock_addr = compute_address (d->m_host_addr, d->m_bytes_sent);
         unsigned long last_page_portion = ((unsigned long)(lock_addr) + lock_size) & ACL_PCIE_DMA_ATT_PAGE_ADDR_MASK;
         
         if (lock_size == ACL_PCIE_DMA_MAX_PINNED_MEM_SIZE && 
             last_page_portion != 0) {
            lock_size -= last_page_portion;
            ACL_VERBOSE_DEBUG (KERN_DEBUG "Doing max pinning would end at %p. Instead, pinning %u bytes from %p to %p",
                      lock_addr + lock_size + last_page_portion, lock_size, lock_addr, lock_addr+lock_size);
         }

         // No active segment of pinned memory - pin one
         assert(!pending_dma_full(aclpci));  

         if (lock_dma_buffer (aclpci, lock_addr, lock_size, &d->m_active_mem) != 0) {
            d->m_update_time += (get_jiffies_64() - startj);
            return 1;
         }
      }

      // As long as the hardware FIFOs are not full, populate them with transfer requests
      while( !hardware_buffers_full(aclpci) && (d->m_active_mem.pages_rem > 0) )
      {
         // unsigned int max_page_size;
         unsigned long pPhysicalAddr;
         unsigned int dwBytes;
         unsigned int unaligned_start = 0;

         struct page *next_page = *(d->m_active_mem.next_page);
         
         // Peek at the next ATT page
         pPhysicalAddr = page_to_phys (next_page);
         
         dwBytes = PAGE_SIZE;
         // First page. If we begin with an offset, we can't use the full page
         if (d->m_active_mem.first_page_offset != 0)
         {
            dwBytes -= d->m_active_mem.first_page_offset;
            pPhysicalAddr += d->m_active_mem.first_page_offset;
            ACL_VERBOSE_DEBUG (KERN_DEBUG "First page. Adjusted bytes to %u. Adjusted PhysAddr to 0x%lx",
                        dwBytes, pPhysicalAddr);
            d->m_active_mem.first_page_offset = 0;
            unaligned_start = 1;
         }
         
         // Last page (could also be the first page.
         if (d->m_active_mem.pages_rem == 1 && d->m_active_mem.last_page_offset != 0)
         {
            dwBytes -= (PAGE_SIZE - d->m_active_mem.last_page_offset);
            ACL_VERBOSE_DEBUG (KERN_DEBUG "Last page. Adjusted bytes to %u.", dwBytes);
         }
         
         ++d->m_active_mem.next_page;
         --d->m_active_mem.pages_rem;

         // If this transfer does not begin on a page boundary, then we need to create a new
         // descriptor for it
         if(d->m_active_descriptor_valid && unaligned_start)
         {
            send_active_descriptor(aclpci);
         }

         // If there is no active descriptor, create a new one
         if(!d->m_active_descriptor_valid)
         {
            unsigned int avalon_address = ACL_PCIE_TX_PORT | (d->m_next_att_row * PAGE_SIZE) | (pPhysicalAddr & ACL_PCIE_DMA_ATT_PAGE_ADDR_MASK);
            
            d->m_active_descriptor.read_address = d->m_read ? (d->m_device_addr + d->m_bytes_sent) : avalon_address;
            d->m_active_descriptor.write_address = d->m_read ? avalon_address : (d->m_device_addr + d->m_bytes_sent);
            d->m_active_descriptor.bytes = 0;
            d->m_active_descriptor_size = 0;
            d->m_active_descriptor_valid = true;
         }

         // Write out this ATT page, add it to the descriptor, update the bytes sent counter
         set_att_entry(aclpci, pPhysicalAddr, d->m_next_att_row );
         d->m_next_att_row = (d->m_next_att_row + 1) % ACL_PCIE_DMA_MAX_ATT_SIZE;
         ++d->m_active_descriptor_size;
         d->m_active_descriptor.bytes += (unsigned int)(dwBytes);
         d->m_bytes_sent += dwBytes;

         // If this descriptor cannot be merged with the next ATT page, send it now
         if( (((pPhysicalAddr + dwBytes) & ACL_PCIE_DMA_ATT_PAGE_ADDR_MASK) != 0) ||
             (d->m_next_att_row == 0) ||
             (d->m_active_descriptor_size == ACL_PCIE_DMA_MAX_ATT_PER_DESCRIPTOR) )
         {
            send_active_descriptor(aclpci);
         }
      }

      // If we finished the active page, push it into the list of pending DMAs
      if( d->m_active_mem.pages_rem == 0 )
      {
         if(d->m_active_descriptor_valid && !hardware_buffers_full(aclpci))
         {
            send_active_descriptor(aclpci);
         }
         queue_push (&d->m_dma_pending, &(d->m_active_mem.dma));
         d->m_active_mem.dma.ptr = NULL;
      }
      ACL_VERBOSE_DEBUG (KERN_DEBUG  "DMA: %d pages remain.", d->m_active_mem.pages_rem);
   }

   // There may be one last descriptor to send
   if(d->m_active_descriptor_valid && !hardware_buffers_full(aclpci))
   {
      send_active_descriptor(aclpci);
   }

   d->m_update_time += (get_jiffies_64() - startj);
   return 1;
}


void set_att_entry(struct aclpci_dev *aclpci, unsigned long address, unsigned int row )
{
  struct aclpci_dma *d = &(aclpci->dma_data);
  int is64bit = ((address & 0xffffffff00000000ULL) != 0ULL);
  unsigned long write_addr = PCIE_CRA_ADDR_TRANS + row * 8;

  if(is64bit)
  {
     ACL_VERBOSE_DEBUG (KERN_DEBUG "DMA: Set 64bit-ATT[%u] = 0x%p\n", row, (void*)address );
     pcie_cra_write64 (aclpci, (void*)write_addr, ((address & 0xfffffffffffffffcULL) | 0x1) );
  }
  else
  {
     ACL_VERBOSE_DEBUG (KERN_DEBUG "DMA: Set 32bit-ATT[%u] = 0x%p\n", row, (void*)address );
     pcie_cra_write32 (aclpci, (void*)write_addr, ((unsigned int)(address) & 0x00000000fffffffcULL) );
  }
  ++d->m_att_size;
}

void send_active_descriptor(struct aclpci_dev *aclpci)
{
   struct DESCRIPTOR_UPDATE_DATA desc;
   struct aclpci_dma *d = &(aclpci->dma_data);

   // DMA controller is setup to only handle aligned requests - verify this is a 256-bit aligned request
   // If this fails, I think we need to bite the bullet and write our own DMA
   if(((d->m_active_descriptor.read_address & DMA_ALIGNMENT_BYTE_MASK) != 0) ||
      ((d->m_active_descriptor.write_address & DMA_ALIGNMENT_BYTE_MASK) != 0) ||
      ((d->m_active_descriptor.bytes & DMA_ALIGNMENT_BYTE_MASK) != 0) )
   {
      ACL_DEBUG (KERN_WARNING "Error: Attempted to send unaligned descriptor\n");
      ACL_DEBUG (KERN_WARNING "       0x%u -> 0x%u (%u ATT entries, 0x%u bytes)\n", 
              d->m_active_descriptor.read_address,
              d->m_active_descriptor.write_address,
              d->m_active_descriptor_size, 
              d->m_active_descriptor.bytes);
      assert(0);
   }

   assert(d->m_active_descriptor_valid);
   d->m_active_descriptor.control = (unsigned int)( ACL_PCIE_GET_BIT(DMA_DC_GO) | 
                                   ACL_PCIE_GET_BIT(DMA_DC_EARLY_DONE_ENABLE) |
                                   ACL_PCIE_GET_BIT(DMA_DC_TRANSFER_COMPLETE_IRQ_MASK) );
   wmb();
   dma_desc_write (aclpci, 0, &d->m_active_descriptor);
   
   d->m_descriptors_sent++;
   ACL_VERBOSE_DEBUG (KERN_DEBUG "DMA #%d: Sent descriptor 0x%x -> 0x%x\n"
                                 "(%d ATT entries, %u bytes, %x control)\n", 
                     d->m_descriptors_sent,
                     d->m_active_descriptor.read_address,
                     d->m_active_descriptor.write_address,
                     d->m_active_descriptor_size, 
                     d->m_active_descriptor.bytes,
                     d->m_active_descriptor.control);
   
   desc.bytes = d->m_active_descriptor.bytes;
   desc.att_entries = d->m_active_descriptor_size;
   queue_push (&d->m_desc_pending, &desc);
   d->m_active_descriptor_valid = false;
}


int read_write
(
   struct aclpci_dev *aclpci, 
   void* src,
   void *dst,
   size_t bytes,
   int reading
)
{  
   unsigned long dev_addr;
   struct aclpci_dma *d = &(aclpci->dma_data);
   
   // TODO: For now, only handle one transfer at a time
   assert(queue_empty (&d->m_dma_pending));
   assert(queue_empty (&d->m_desc_pending));
   assert(d->m_active_mem.dma.ptr == NULL);
   assert(!d->m_active_descriptor_valid);
   assert(d->m_att_size == 0);

   // Copy the parameters over and mark the job as running
   d->m_read = reading;
   d->m_bytes = (unsigned int)(bytes);
   assert(d->m_bytes == bytes);
   d->m_host_addr = reading ? dst : src;
   dev_addr = (unsigned long)(reading ? src : dst);
   d->m_device_addr = (unsigned int)(dev_addr);
   assert(dev_addr < UINT_MAX);
   d->m_idle = 0;

   // Start processing the request
   d->m_bytes_sent = 0;
   d->m_next_att_row = 0;
   d->m_descriptors_acknowledged = 0;
   d->m_descriptors_sent = 0;
   d->m_descriptors_updated = 0;
   d->m_bytes_acknowledged = 0;
   
   d->m_update_time = 0;
   d->m_pin_time = d->m_lock_time = d->m_unlock_time = 0;
   d->m_start_time = get_jiffies_64();
   
   aclpci_dma_update (aclpci, 1);
    
   return 1;
}

#else // USE_DMA is 0

irqreturn_t aclpci_dma_service_interrupt (void) {
  return IRQ_HANDLED;
}
ssize_t aclpci_dma_rw (void *dev_addr, void __user* user_addr, 
                       ssize_t len, int reading) {return 0; }
void aclpci_dma_init(struct aclpci_dev *aclpci) {}
void aclpci_dma_finish(void) {}
int aclpci_dma_get_idle_status(void) { return 1; }
int aclpci_dma_update(int forced) { return 0; }

#endif // USE_DMA
