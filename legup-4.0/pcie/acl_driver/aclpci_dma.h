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

/* Defines used only by aclpci_dma.c. */


#if USE_DMA

/* Enable Linux-specific defines in the hw_pcie_dma.h file */
#define LINUX
#include "hw_pcie_dma.h"
#include "aclpci_queue.h"

struct dma_t {
  void *ptr;         /* if ptr is NULL, the whole struct considered invalid */
  size_t len;
  enum dma_data_direction dir;
  struct page **pages;     /* one for each struct page */
  dma_addr_t *dma_addrs;   /* one for each struct page */
  unsigned int num_pages;
};

struct pinned_mem {
  struct dma_t dma;
  struct page **next_page;
  unsigned int pages_rem;
  unsigned int first_page_offset;
  unsigned int last_page_offset;
};


struct aclpci_dma {

  // Update information
  unsigned int m_descriptors_updated;
  unsigned int m_descriptors_acknowledged;
  unsigned int m_descriptors_sent;
  unsigned int m_old_done_count;
  unsigned int m_bytes_acknowledged;

  // The container of pending memory transactions.
  // Contains wd_dma values
  struct queue m_dma_pending;

  // A representation of the hardware's descriptor fifo
  // Contains DESCRIPTOR_UPDATE_DATA values
  struct queue m_desc_pending;

  // Pinned memory we're currently building DMA transactions for.
  struct pinned_mem m_active_mem;

  // The transaction we are currently working on
  struct DMA_DESCRIPTOR m_active_descriptor;

  int m_active_descriptor_valid;
  unsigned int m_active_descriptor_size;
  // The next ATT table row to write to
  unsigned int m_next_att_row;
  // The total number of active ATT rows
  unsigned int m_att_size;

  struct pci_dev *m_pci_dev;
  struct aclpci_dev *m_aclpci;

  // Transfer information
  unsigned int m_device_addr;
  void* m_host_addr;
  int m_read;
  unsigned int m_bytes;
  unsigned int m_bytes_sent;
  int m_idle;

  u64 m_update_time, m_pin_time, m_start_time;
  u64 m_lock_time, m_unlock_time;
};

#else
struct aclpci_dma {};
#endif
