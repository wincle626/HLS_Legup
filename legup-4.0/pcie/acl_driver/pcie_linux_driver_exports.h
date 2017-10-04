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

/* All defines necessary to communicate with the Linux PCIe driver.
 * The actual communication functions are open()/close()/read()/write().
 *
 * Example read call (read single ushort from BAR 0, device address 0x2):
 *   ssize_t f = open ("/dev/de4", O_RDWR);
 *   unsigned short val;
 *   struct aclpci_cmd read_cmd = { 0, ACLPCI_CMD_DEFAULT, 0x2, &val };
 *   read (f, &read_cmd, sizeof(val));
 *
 * See user.c for a tester of all functions and more elaborate examples.
 */

#ifndef PCIE_LINUX_DRIVER_EXPORTS_H
#define PCIE_LINUX_DRIVER_EXPORTS_H


/* if bar_id in aclpci_cmd is set to this, this is a special command,
 * not a usual read/write request. So the command field is used. Otherwise,
 * command field is ignored. */
#define ACLPCI_CMD_BAR 23

/* Values for 'command' field of aclpci_cmd. */

/* Default value -- noop. */
#define ACLPCI_CMD_DEFAULT                0

/* Save/Restore all board PCI control registers to user_addr.
 * Allows user program to reprogram the board without having root
 * priviliges (which is required to change PCI control registers). */
#define ACLPCI_CMD_SAVE_PCI_CONTROL_REGS  1
#define ACLPCI_CMD_LOAD_PCI_CONTROL_REGS  2

/* Lock/Unlock user_addr memory to physical RAM ("pin" it) */
#define ACLPCI_CMD_PIN_USER_ADDR          3
#define ACLPCI_CMD_UNPIN_USER_ADDR        4

/* Get m_idle status of DMA */
#define ACLPCI_CMD_GET_DMA_IDLE_STATUS    5
#define ACLPCI_CMD_DMA_UPDATE             6

/* Get vendor_id and device_id of loaded PCIe device */
#define ACLPCI_CMD_GET_DEVICE_ID          7
#define ACLPCI_CMD_GET_VENDOR_ID          8

/* Change FPGA core image by using CvP.
 * The caller must provide the .core.rbf file loaded into memory */
#define ACLPCI_CMD_DO_CVP                 9

/* PCIe link status queries (PCIe gen and number of lanes) */
#define ACLPCI_CMD_GET_PCI_GEN            10
#define ACLPCI_CMD_GET_PCI_NUM_LANES      11

/* Set id to receive back on signal from kernel */
#define ACLPCI_CMD_SET_SIGNAL_PAYLOAD     12

/* Get full driver version, as string */
#define ACLPCI_CMD_GET_DRIVER_VERSION     13

#define ACLPCI_CMD_MAX_CMD                14


/* Signal from driver to user (hal) to notify about hw interrupt */
#define SIG_INT_NOTIFY 44

/* Main structure to communicate any command (including read/write)
 * from user space to the driver. */
struct aclpci_cmd {

  /* base address register of PCIe device. device_addr is interpreted
   * as an offset from this BAR's start address. */
  unsigned int bar_id;
  
  /* Special command to execute. Only used if bar_id is set
   * to ACLPCI_CMD_BAR. */
  unsigned int command;
  
  /* Address in device space where to read/write data. */
  void* device_addr;
  
  /* Address in user space where to write/read data.
   * Always virtual address. */
  void* user_addr;
  
};

#endif /* PCIE_LINUX_DRIVER_EXPORTS_H */
