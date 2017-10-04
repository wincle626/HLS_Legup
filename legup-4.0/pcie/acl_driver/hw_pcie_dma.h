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

#ifndef HW_PCIE_DMA_H
#define HW_PCIE_DMA_H


// DMA parameters to tweak
static const unsigned int ACL_PCIE_DMA_MAX_PINNED_MEM_SIZE = 1024*1024;
static const unsigned int ACL_PCIE_DMA_MAX_PINNED_MEM = 64; // x PINNED_MEM_SIZE above
static const unsigned int ACL_PCIE_DMA_MAX_ATT_PER_DESCRIPTOR = 128;

// Constants matched to the HW
static const unsigned int ACL_PCIE_DMA_MAX_DONE_COUNT = (1 << 16);
static const unsigned int ACL_PCIE_DMA_MAX_ATT_PAGE_SIZE = 4*1024;
static const unsigned int ACL_PCIE_DMA_MAX_ATT_SIZE = 256;
static const unsigned int ACL_PCIE_DMA_MAX_DESCRIPTORS = 128;

static const unsigned int ACL_PCIE_DMA_ATT_PAGE_ADDR_MASK = 4*1024-1; // (ACL_PCIE_DMA_MAX_ATT_PAGE_SIZE-1);

#ifdef LINUX
#  define cl_ulong unsigned long
#endif

struct DMA_DESCRIPTOR {
   unsigned int read_address;
   unsigned int write_address;
   unsigned int bytes;
   unsigned int control;
};

struct DESCRIPTOR_UPDATE_DATA {
   unsigned int bytes;
   unsigned int att_entries;
   cl_ulong start;
};

#endif // HW_PCIE_DMA_H
