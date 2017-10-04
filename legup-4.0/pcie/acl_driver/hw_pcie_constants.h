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

////////////////////////////////////////////////////////////
//                                                        //
// hw_pcie_constants.h                                    //
// Constants to keep in sync with the HW board design     //
//                                                        //
// Note: This file *MUST* be kept in sync with any        //
//       changes to the HW board design!                  //
//                                                        //
////////////////////////////////////////////////////////////

#ifndef HW_PCIE_CONSTANTS_H
#define HW_PCIE_CONSTANTS_H


struct board_parameters {
   const char * board_name;
   unsigned int acl_pci_global_mem_bar;
   unsigned int acl_pci_cra_bar;
   unsigned int acl_pci_cra_offset;
   unsigned int acl_pci_cra_size;
   unsigned int acl_kernel_csr_bar;
   unsigned int acl_kernel_csr_offset;
   unsigned int acl_kernel_option0_csr_size;
   unsigned int acl_configuration_storage_bar;
   unsigned int acl_configuration_storage_offset;
   unsigned int acl_pcie_dma_bar;
   unsigned int acl_pcie_dma_offset;
   unsigned int acl_pcie_dma_descriptor_bar;
   unsigned int acl_pcie_dma_descriptor_offset;
   unsigned int acl_pcie_pio_in_bar;
   unsigned int acl_pcie_pio_in_offset;
   unsigned int acl_pcie_pio_out_bar;
   unsigned int acl_pcie_pio_out_offset;
   unsigned int acl_pcie_tx_port;
   unsigned int acl_pcie_memwindow_bar;
   unsigned int acl_pcie_memwindow_cra;
   unsigned int acl_pcie_memwindow_base;
   unsigned int acl_pcie_memwindow_size;
   unsigned int acl_pcie_kernwindow_bar;
   unsigned int acl_pcie_kernwindow_cra;
   unsigned int acl_pcie_kernwindow_base;
   unsigned int acl_pcie_kernwindow_size;
   unsigned int acl_pcie_em_bar;
   unsigned int acl_pcie_em_offset;
   unsigned int acl_pcie_em2_bar;
   unsigned int acl_pcie_em2_offset;
   // Byte offsets
   unsigned int kernel_offset_csr;
   unsigned int kernel_offset_printf_buffer_size;
   unsigned int kernel_offset_invocation_image;
   unsigned int kernel_csr_go;
   unsigned int kernel_csr_done;
   unsigned int kernel_csr_stalled;
   unsigned int kernel_csr_unstall;
   unsigned int kernel_csr_last_status_bit;
   unsigned int kernel_csr_status_bits_mask;
   unsigned int kernel_csr_lmem_invalid_bank;
   unsigned int kernel_csr_lsu_active;
   unsigned int kernel_csr_wr_active;
   unsigned int kernel_csr_valid_in;
   unsigned int kernel_csr_started;
   unsigned int kernel_csr_first_version_bit;
   unsigned int kernel_csr_last_version_bit;
   unsigned int pcie_cra_irq_status;
   unsigned int pcie_cra_irq_enable;
   unsigned int pcie_cra_addr_trans;
   unsigned int acl_pcie_kernel_irq_vec;
   unsigned int acl_pcie_dma_irq_vec;
   unsigned int use_kernelpll_reconfig;
   unsigned int acl_pcie_kernelpll_reconfig_bar;
   unsigned int acl_pcie_kernelpll_reconfig_offset;
   unsigned int acl_pcie_kernelpll_rom_bar;
   unsigned int acl_pcie_kernelpll_rom_offset;
   unsigned int acl_pcie_counter_bar;
   unsigned int acl_pcie_counter_offset;
#ifndef QSYS_IFACE
   unsigned int pcie_cra_irq_rxmirq;
   unsigned int pcie_cra_avl_irq_vec_lo;
   unsigned int pcie_cra_avl_irq_vec_hi;
#endif
   unsigned int dma_alignment_bytes;
   unsigned int dma_dc_transfer_complete_irq_mask;
   unsigned int dma_dc_early_done_enable;
   unsigned int dma_dc_go;
   unsigned int dma_csr_status;
   unsigned int dma_csr_control;
   unsigned int dma_status_busy;
   unsigned int dma_status_descriptor_empty;
   unsigned int dma_status_resetting;
   unsigned int dma_status_irq;
   unsigned int dma_status_count_lo;
   unsigned int dma_status_count_hi;
   unsigned int dma_ctrl_stop;
   unsigned int dma_ctrl_reset;
   unsigned int dma_ctrl_irq_enable;
   unsigned int perfmon_status_reg;
   unsigned int perfmon_build_number;
   unsigned int perfmon_perf_registers;
   unsigned int perfmon_log_start_reg;
   unsigned int perfmon_log_end_reg;
   unsigned int perfmon_log_registers;
   unsigned int perfmon_trigger_fifo_register;
   unsigned int perfmon_enable_monitoring;
   unsigned int perfmon_enable_logging;
   unsigned int perfmon_buffer_is_empty;
   unsigned int perfmon_reset;
   unsigned int perfmon_trigger_log_on_fifo_activity;
   unsigned int pio_data;
   unsigned int pio_set;
   unsigned int pio_clr;
   unsigned int pio_out_swreset;
   unsigned int pio_out_pllreset;
   unsigned int pio_out_interleave_mode;
   unsigned int pio_in_kpll_locked;
   unsigned int pio_in_a_init_done;
   unsigned int pio_in_a_cal_success;
   unsigned int pio_in_b_init_done;
   unsigned int pio_in_b_cal_success;
   unsigned int has_temp_sensor;
   unsigned int temp_sensor_addr;
};

enum BOARD_ID {
  DE4 = 0,          //DE4 Board
  DE4_EXPCARD = 1,  //DE4 Board on express card extension board for laptops
  PCIE385 = 2,      //Nallatech SV board
  BSP = 3,           //BSP SV board
  C5DK = 4,
  NUM_SUPPORTED_BOARDS = 5
};


#define ACL_PCIE_READ_BIT( w, b ) (((w) >> (b)) & 1)
#define ACL_PCIE_READ_BIT_RANGE( w, h, l ) (((w) >> (l)) & ((1 << ((h) - (l) + 1)) - 1))
#define ACL_PCIE_SET_BIT( w, b ) ((w) |= (1 << (b)))
#define ACL_PCIE_CLEAR_BIT( w, b ) ((w) &= (~(1 << (b))))
#define ACL_PCIE_GET_BIT( b ) (unsigned) (1 << (b))

#define QSYS_IFACE 1
// Number of Base Address Registers in the PCIe core
#define ACL_PCI_NUM_BARS 4

// PCI Vendor and Device IDs
#define ACL_PCI_ALTERA_VENDOR_ID            0x1172
#define ACL_PCI_DE4_DEVICE_ID               0XDE4 
#define ACL_PCI_DE4_EXPCARD_DEVICE_ID       0XEDE4 
#define ACL_PCI_PCIE385_DEVICE_ID           0xE385
#define ACL_PCI_BSP_DEVICE_ID               0xAB00

#define ACL_SOC_C5_DK                       0xC5DE
#define ACL_CONFIGURATION_STORAGE_SIZE      4096 

#define ACL_BOARD_NAME boards[board_id].board_name

// Global memory
#define ACL_PCI_GLOBAL_MEM_BAR boards[board_id].acl_pci_global_mem_bar 

// PCIe control register addresses
#define ACL_PCI_CRA_BAR  boards[board_id].acl_pci_cra_bar  
#define ACL_PCI_CRA_OFFSET  boards[board_id].acl_pci_cra_offset  
#define ACL_PCI_CRA_SIZE boards[board_id].acl_pci_cra_size 

// Kernel control/status register addresses
#define ACL_KERNEL_CSR_BAR boards[board_id].acl_kernel_csr_bar 
#define ACL_KERNEL_CSR_OFFSET boards[board_id].acl_kernel_csr_offset 
#define ACL_KERNEL_OPTION0_CSR_SIZE boards[board_id].acl_kernel_option0_csr_size 

// Location of the auto-discover configuration storage information
#define ACL_CONFIGURATION_STORAGE_BAR boards[board_id].acl_configuration_storage_bar 
#define ACL_CONFIGURATION_STORAGE_OFFSET boards[board_id].acl_configuration_storage_offset 

// DMA control/status register address
#define ACL_PCIE_DMA_BAR boards[board_id].acl_pcie_dma_bar 
#define ACL_PCIE_DMA_OFFSET boards[board_id].acl_pcie_dma_offset 

// DMA descriptor slave address
#define ACL_PCIE_DMA_DESCRIPTOR_BAR boards[board_id].acl_pcie_dma_descriptor_bar 
#define ACL_PCIE_DMA_DESCRIPTOR_OFFSET boards[board_id].acl_pcie_dma_descriptor_offset 

// PIO input slave address
#define ACL_PCIE_PIO_IN_BAR boards[board_id].acl_pcie_pio_in_bar 
#define ACL_PCIE_PIO_IN_OFFSET boards[board_id].acl_pcie_pio_in_offset

// PIO output slave address
#define ACL_PCIE_PIO_OUT_BAR boards[board_id].acl_pcie_pio_out_bar 
#define ACL_PCIE_PIO_OUT_OFFSET boards[board_id].acl_pcie_pio_out_offset

// Avalon Tx port address as seen by the DMA read/write masters
#define ACL_PCIE_TX_PORT boards[board_id].acl_pcie_tx_port 

// Global memory window slave address.  The host has different "view" of global
// memory: it sees only 512megs segments of memory at a time for non-DMA xfers
#define ACL_PCIE_MEMWINDOW_BAR boards[board_id].acl_pcie_memwindow_bar 
#define ACL_PCIE_MEMWINDOW_CRA boards[board_id].acl_pcie_memwindow_cra 
#define ACL_PCIE_MEMWINDOW_BASE boards[board_id].acl_pcie_memwindow_base 
#define ACL_PCIE_MEMWINDOW_SIZE boards[board_id].acl_pcie_memwindow_size 

// Kernel window slave address.  A 4K window is also used for the kernel
// CRA addresses.  This lets us scale # of kernels and debug ports without 
// requiring more address bits out of the iface
#define ACL_PCIE_KERNWINDOW_BAR boards[board_id].acl_pcie_kernwindow_bar 
#define ACL_PCIE_KERNWINDOW_CRA boards[board_id].acl_pcie_kernwindow_cra 
#define ACL_PCIE_KERNWINDOW_BASE boards[board_id].acl_pcie_kernwindow_base 
#define ACL_PCIE_KERNWINDOW_SIZE boards[board_id].acl_pcie_kernwindow_size 

// Efficiency Monitor address
#define ACL_PCIE_EM_BAR boards[board_id].acl_pcie_em_bar 
#define ACL_PCIE_EM_OFFSET boards[board_id].acl_pcie_em_offset 

// Efficiency Monitor address for 2nd DIMM
#define ACL_PCIE_EM2_BAR boards[board_id].acl_pcie_em2_bar 
#define ACL_PCIE_EM2_OFFSET boards[board_id].acl_pcie_em2_offset 

// Byte offsets
#define KERNEL_OFFSET_CSR boards[board_id].kernel_offset_csr
#define KERNEL_OFFSET_PRINTF_BUFFER_SIZE boards[board_id].kernel_offset_printf_buffer_size
#define KERNEL_OFFSET_INVOCATION_IMAGE boards[board_id].kernel_offset_invocation_image

// Bits in the kernel CSR register
// Bits 16-31 contain a version code that can be read;writes to those bits are ignored.
// Option 3 wrappers have version 1.
// Prior to Option 3, bits 16-31 always read as 0.
#define KERNEL_CSR_GO boards[board_id].kernel_csr_go
#define KERNEL_CSR_DONE boards[board_id].kernel_csr_done
#define KERNEL_CSR_STALLED boards[board_id].kernel_csr_stalled
#define KERNEL_CSR_UNSTALL boards[board_id].kernel_csr_unstall
#define KERNEL_CSR_LAST_STATUS_BIT boards[board_id].kernel_csr_last_status_bit
#define KERNEL_CSR_STATUS_BITS_MASK boards[board_id].kernel_csr_status_bits_mask
#define KERNEL_CSR_LMEM_INVALID_BANK boards[board_id].kernel_csr_lmem_invalid_bank
#define KERNEL_CSR_LSU_ACTIVE boards[board_id].kernel_csr_lsu_active
#define KERNEL_CSR_WR_ACTIVE boards[board_id].kernel_csr_wr_active
#define KERNEL_CSR_VALID_IN boards[board_id].kernel_csr_valid_in
#define KERNEL_CSR_STARTED boards[board_id].kernel_csr_started
#define KERNEL_CSR_FIRST_VERSION_BIT boards[board_id].kernel_csr_first_version_bit
#define KERNEL_CSR_LAST_VERSION_BIT boards[board_id].kernel_csr_last_version_bit

// PCI express control-register offsets
#define PCIE_CRA_IRQ_STATUS boards[board_id].pcie_cra_irq_status
#define PCIE_CRA_IRQ_ENABLE boards[board_id].pcie_cra_irq_enable
#define PCIE_CRA_ADDR_TRANS boards[board_id].pcie_cra_addr_trans

// IRQ vector mappings (as seen by the PCIe RxIRQ port)
#define ACL_PCIE_KERNEL_IRQ_VEC boards[board_id].acl_pcie_kernel_irq_vec
#define ACL_PCIE_DMA_IRQ_VEC boards[board_id].acl_pcie_dma_irq_vec

// PLL related
#define USE_KERNELPLL_RECONFIG boards[board_id].use_kernelpll_reconfig 
#define ACL_PCIE_KERNELPLL_RECONFIG_BAR boards[board_id].acl_pcie_kernelpll_reconfig_bar 
#define ACL_PCIE_KERNELPLL_RECONFIG_OFFSET boards[board_id].acl_pcie_kernelpll_reconfig_offset
#define ACL_PCIE_KERNELPLL_ROM_BAR boards[board_id].acl_pcie_kernelpll_rom_bar 
#define ACL_PCIE_KERNELPLL_ROM_OFFSET boards[board_id].acl_pcie_kernelpll_rom_offset
#define ACL_PCIE_COUNTER_BAR boards[board_id].acl_pcie_counter_bar 
#define ACL_PCIE_COUNTER_OFFSET boards[board_id].acl_pcie_counter_offset

#ifndef QSYS_IFACE
// PCI express IRQ register bits
#define PCIE_CRA_IRQ_RXMIRQ boards[board_id].pcie_cra_irq_rxmirq
#define PCIE_CRA_AVL_IRQ_VEC_LO boards[board_id].pcie_cra_avl_irq_vec_lo
#define PCIE_CRA_AVL_IRQ_VEC_HI boards[board_id].pcie_cra_avl_irq_vec_hi
#endif

// DMA descriptor control bits
#define DMA_ALIGNMENT_BYTES boards[board_id].dma_alignment_bytes
#define DMA_ALIGNMENT_BYTE_MASK (DMA_ALIGNMENT_BYTES-1)
#define DMA_DC_TRANSFER_COMPLETE_IRQ_MASK boards[board_id].dma_dc_transfer_complete_irq_mask
#define DMA_DC_EARLY_DONE_ENABLE boards[board_id].dma_dc_early_done_enable
#define DMA_DC_GO boards[board_id].dma_dc_go
// DMA controller control/status registers
#define DMA_CSR_STATUS boards[board_id].dma_csr_status
#define DMA_CSR_CONTROL boards[board_id].dma_csr_control
// DMA CSR status bits
#define DMA_STATUS_BUSY boards[board_id].dma_status_busy
#define DMA_STATUS_DESCRIPTOR_EMPTY boards[board_id].dma_status_descriptor_empty
#define DMA_STATUS_RESETTING boards[board_id].dma_status_resetting
#define DMA_STATUS_IRQ boards[board_id].dma_status_irq
#define DMA_STATUS_COUNT_LO boards[board_id].dma_status_count_lo
#define DMA_STATUS_COUNT_HI boards[board_id].dma_status_count_hi
// DMA CSR control bits
#define DMA_CTRL_STOP boards[board_id].dma_ctrl_stop
#define DMA_CTRL_RESET boards[board_id].dma_ctrl_reset
#define DMA_CTRL_IRQ_ENABLE boards[board_id].dma_ctrl_irq_enable
// Kernel performance monitor control/status registers
// The Avalon slave data is 32-bits while PCI address is w.r.t. bytes, so *4 to translate
#define PERFMON_STATUS_REG boards[board_id].perfmon_status_reg
#define PERFMON_BUILD_NUMBER boards[board_id].perfmon_build_number
#define PERFMON_PERF_REGISTERS boards[board_id].perfmon_perf_registers
#define PERFMON_LOG_START_REG boards[board_id].perfmon_log_start_reg
#define PERFMON_LOG_END_REG boards[board_id].perfmon_log_end_reg
#define PERFMON_LOG_REGISTERS boards[board_id].perfmon_log_registers
#define PERFMON_TRIGGER_FIFO_REGISTER boards[board_id].perfmon_trigger_fifo_register
// Kernel performance monitor status/control bits
#define PERFMON_ENABLE_MONITORING boards[board_id].perfmon_enable_monitoring
#define PERFMON_ENABLE_LOGGING boards[board_id].perfmon_enable_logging
#define PERFMON_BUFFER_IS_EMPTY boards[board_id].perfmon_buffer_is_empty
#define PERFMON_RESET boards[board_id].perfmon_reset
#define PERFMON_TRIGGER_LOG_ON_FIFO_ACTIVITY boards[board_id].perfmon_trigger_log_on_fifo_activity
#define PIO_DATA boards[board_id].pio_data
#define PIO_SET boards[board_id].pio_set
#define PIO_CLR boards[board_id].pio_clr
#define PIO_OUT_SWRESET boards[board_id].pio_out_swreset
#define PIO_OUT_PLLRESET boards[board_id].pio_out_pllreset
#define PIO_OUT_INTERLEAVE_MODE boards[board_id].pio_out_interleave_mode
#define PIO_IN_KPLL_LOCKED boards[board_id].pio_in_kpll_locked
#define PIO_IN_A_INIT_DONE boards[board_id].pio_in_a_init_done
#define PIO_IN_A_CAL_SUCCESS boards[board_id].pio_in_a_cal_success
#define PIO_IN_B_INIT_DONE boards[board_id].pio_in_b_init_done
#define PIO_IN_B_CAL_SUCCESS boards[board_id].pio_in_b_cal_success

// Temperature sensor presence and base address macros
#define ACL_PCIE_HAS_TEMP_SENSOR boards[board_id].has_temp_sensor 
#define ACL_PCIE_TEMP_SENSOR_ADDRESS boards[board_id].temp_sensor_addr 

extern int board_id;
extern struct board_parameters boards[NUM_SUPPORTED_BOARDS];

#endif // HW_PCIE_CONSTANTS_H
