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

/* Reprogram the board with given SOF. */
 
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include "pcie_linux_driver_exports.h"

/* Contains common util functions between Linux and Windows version of reprogram */
#include "reprogram_common.c"
#include "version.h"

#define DEBUG 0

int reprogram_with_sof (ssize_t f, const char *filename);
int reprogram_with_rbf (ssize_t f, const char *filename);
int reprogram_core_with_CvP (ssize_t f, const char *filename);
int test_global_mem (ssize_t f);
int test_auto_discovery_read (ssize_t f);
int rebuild_driver();

unsigned int get_value_from_driver(ssize_t f, unsigned int ACLPCI_CMD_VAL) {
  unsigned int res = 0;
  struct aclpci_cmd read_cmd = { ACLPCI_CMD_BAR, 
                                 ACLPCI_CMD_VAL, 
                                 NULL, 
                                 &res };
  read (f, &read_cmd, 0);
  return res;
}


/* Return 1 if the driver was re-loaded. The caller must re-open the device */
int check_driver_version (ssize_t f) {

  char buf[128] = {0};
  struct aclpci_cmd read_cmd = { ACLPCI_CMD_BAR, 
                                 ACLPCI_CMD_GET_DRIVER_VERSION, 
                                 NULL, 
                                 &buf };
  read (f, &read_cmd, 0);
  printf ("Driver version: %s\n", buf);
  if (strcmp (buf, ACL_DRIVER_VERSION) != 0) {
    printf ("reprogram was compiled for driver version: %s\n", ACL_DRIVER_VERSION);
    
    /* If have /etc/aclpci_reload_driver, recompile and reload the driver automatically.
     * Otherwise, just complain but keep going. */
    FILE *reloader = fopen ("/etc/aclpci_reload_driver", "r");
    if (reloader == 0) {
      printf ("The driver version does not match reprogram version!\n");
      printf ("Please update the driver.\n");
      return 0;
      
    } else {
      fclose (reloader);
      printf ("The driver version does not match reprogram version! Re-building and re-loading the driver.\n");
      close (f);
      rebuild_driver();
      return 1; /* The caller will re-open the device */
    }
  } else {
    printf ("The driver version is up-to-date\n");
    return 0;
  }
}


int rebuild_driver() {

  FILE *fp;
  char path[1024] = {0};

  /* Do 'which reprogram' and capture the output. */
  fp = popen("which reprogram", "r");
  if (fp == 0) {
    printf ("Failed to run command\n" );
    return 1;
  }

  /* Read the output a line at a time - output it. */
  while (fgets(path, sizeof(path)-1, fp) != NULL) {
    if (strstr(path, "acl/linux64") != 0) {
      break;
    }
  }
  /* close the pipe*/
  pclose(fp);
  
  if (path[0] == 0) {
    printf ("Could not find location of linux driver sources!\n");
    return 1;
  }
  
  char *bin_loc = strstr (path, "bin/reprogram");
  if (bin_loc == 0) {
    printf ("Could not find location of linux driver sources!\n");
    return 1;
  }
  *bin_loc = '\0';
  
  strcat (path, "driver");
  printf("Driver source code is here: %s\n", path);
  
  char cmd[1024];
  sprintf (cmd, "cd %s; sh make_all.sh; /etc/aclpci_reload_driver aclpci_drv.ko", path);
  printf ("Executing: %s\n", cmd);
  system (cmd);

  return 0;
}


/* Print link stats */
void print_link_stats(ssize_t f) {

  printf ("Link is operating as PCIe gen%d x %d\n", 
          get_value_from_driver (f, ACLPCI_CMD_GET_PCI_GEN),
          get_value_from_driver (f, ACLPCI_CMD_GET_PCI_NUM_LANES));
          
  printf ("Vendor id = 0x%x, device id = 0x%x\n", 
          get_value_from_driver (f, ACLPCI_CMD_GET_VENDOR_ID),
          get_value_from_driver (f, ACLPCI_CMD_GET_DEVICE_ID));
}


static int state_saved = 0;
void save_pcie_state (ssize_t f) {

  /* Command struct to pass to the driver */
  struct aclpci_cmd read_cmd = { ACLPCI_CMD_BAR, ACLPCI_CMD_SAVE_PCI_CONTROL_REGS, NULL, NULL };
  
  printf ("Saving PCI control registers of the board.\n");
  read (f, &read_cmd, 0);
  state_saved = 1;
}

void restore_pcie_state(ssize_t f) {

  struct aclpci_cmd read_cmd = { ACLPCI_CMD_BAR, ACLPCI_CMD_SAVE_PCI_CONTROL_REGS, NULL, NULL };

  printf ("Restoring PCI control registers of the board.\n");
  read_cmd.command = ACLPCI_CMD_LOAD_PCI_CONTROL_REGS;
  read (f, &read_cmd, 0);
  state_saved = 0;
}


int main(int argc, char **argv) {
  
  /* Ignore kernel-completion signal.
   * If a host program was killed right after launching a kernel,
   * the device might keep sending kernel-done signals. That would kill
   * this exe unless we specifically ignore these signals */
  struct sigaction sig;
  sig.sa_sigaction = NULL;
  sig.sa_handler = SIG_IGN;
  sig.sa_flags = SIGEV_NONE;
  sigaction(SIG_INT_NOTIFY, &sig, NULL);
  int ret;

  
  /* Open the device */
  ssize_t f = open ("/dev/de4", O_RDWR);
  if (f == -1) {
    printf ("Couldn't open the device! Did you load the driver?\n");
    return 1;
  }

  ret = check_driver_version (f);
  if (ret) {
    /* Need to re-open the board.*/
    f = open ("/dev/de4", O_RDWR);
    if (f == -1) {
      printf ("Couldn't open the device after driver re-load!\n");
      return 1;
    }
  }
  
  
  if (argc != 2 && argc != 3) {
    printf ("Usage: reprogram <SOF/RBF> [<CORE.RBF>]\n");
    return 1;
  }

  if (strcmp(argv[1], "--manual") == 0) {
    printf("Manual reprogram mode.\n  I'll save the state, you do what you need to do, then press any key to restore the state.\n");
    printf("Link info:\n");
    print_link_stats(f);
    save_pcie_state(f);
    printf("State saved.\n");
    int c;
    printf("Press ENTER to continue...\n");
    do c=getchar(); while((c != '\n') && (c != EOF));
    restore_pcie_state(f);
    printf("State restored.\n");
    printf("Link info:\n");
    print_link_stats(f);
    close (f);
    return 0;
  }
   
  #if DEBUG  
    char buf[1024];
    ssize_t num_read = readlink ("/proc/self", buf, 1024);
    buf[num_read] = '\0';
    printf ("PID of this process: %s\n", buf);
  #endif

  char *filename1 = argv[1];
  char *filename2 = (argc == 3) ? argv[2] : NULL;
  if (!can_read_filename (filename1)) {
    return 1;
  }
  if (filename2) {
    if (!can_read_filename (filename2)) {
      return 1;
    }
  }
     
  const char *sof = NULL;
  const char *core_rbf = NULL;
  const char *bw_rbf = NULL;
  map_filenames_to_sof_and_rbf (filename1, filename2, &sof, &core_rbf, &bw_rbf);

  if (core_rbf == NULL && sof == NULL && bw_rbf == NULL) {
    printf ("Unrecognized file name %s\n", filename1);
    printf ("Only support .sof/.rbf (for full reconfiguration), and\n");
    printf (".core.rbf (for core reconfiguration via PCIe).\n");
    return 1;
  }
  printf ("Given SOF: %s\n", sof);
  printf ("Given RBF: %s\n", bw_rbf);
  printf ("Given core RBF: %s\n", core_rbf);

  save_pcie_state(f);

  /* If have core RBF, try that first. If that fails, fall back to full SOF */
  int cvp_ok = 0;
  if (core_rbf != NULL) {
    cvp_ok = reprogram_core_with_CvP (f, core_rbf);
  }

  if (!cvp_ok && sof != NULL) {
    ret = reprogram_with_sof (f, sof);
  } else if(!cvp_ok && bw_rbf != NULL) {
    ret = reprogram_with_rbf (f, bw_rbf);
  } else if (!cvp_ok) {
    printf ("CvP Failed and full SOF/RBF is not given. Re-run reprogram with full SOF/RBF or reset the board with cold reboot and try CvP again.\n");
    ret = 1;
  } else { /* cvp_ok is true*/
    ret = 0;
  }

  restore_pcie_state(f);
  
  print_link_stats(f);
  close (f);
  return ret;
}

/* reprogram full device using BittWare tookit and RBF */
int reprogram_with_rbf (ssize_t f, const char *filename) {
  int ret;
  printf("Programming the board with new RBF.\n");
  char cmd[4*1024];
  sprintf(cmd, "bwconfig --load=\"%s\",0x203 --type=atlantis --index=0", filename);
  //sprintf(cmd, "bwconfig --load=\"%s\" --type=atlantis --index=0", filename);
  printf ("Executing: %s\n", cmd);
  ret = system (cmd);
  sleep(1); // sleep 1 second

  if(ret != 0)
    printf("Error programming the device.  (Error code %d.)\n", ret);
    
  return ret;
}


/* reprogram full device using quartus_pgm and SOF */  
int reprogram_with_sof (ssize_t f, const char *filename) {

  
  printf ("Programming the board with new SOF.\n");
  char cmd[4*1024];
  sprintf (cmd, "quartus_pgm -c 1 -m jtag -o \"P;%s\"", filename);
  printf ("Executing: %s\n", cmd);
  system (cmd);
  sleep (2); // sleep 2 seconds.
    
  /* quartus_pgm spawns jtagd that holds file handle on the device 
   * (inherits it from here) and allows the device to keep sending
   * bogus interrupts. Killing jtagd will disabled interrupts */
  system ("ps -e -o pid,cmd |grep quartus|grep jtagd|sed \'s/ [\\/[:alpha:]].*//\'|xargs kill -9");
  
  /* this never fails. */
  return 0;
}


/* Change the core only via PCIe.
 * This is supported only for Stratix V and newer devices. */
int reprogram_core_with_CvP (ssize_t f, const char *filename) {

  int result;
  int *core_img = NULL;
  size_t file_size;
  struct aclpci_cmd read_cmd = { 
        ACLPCI_CMD_BAR, 
        ACLPCI_CMD_DO_CVP, 
        NULL, 
        NULL };

  /* Make sure the auto-discovery version number is valid.
   * Non-CvP SOFs will have old version number (failing this check).
   * Also, FPGA in inconsistent state is not worth doing CvP on. */
  if (!test_auto_discovery_read(f)) {
    printf ("Not doing CvP on FPGA with version below 11!\n");
    return 0;
  }

  core_img =  (int*)acl_loadFileIntoMemory (filename, &file_size);
  if (core_img == NULL) {
    return 0;
  }
  /* file_size is in bytes */
  assert (file_size % 4 == 0);
  file_size = file_size/4;

  read_cmd.user_addr = core_img;
  printf ("Starting CvP reprogramming of the device...\n");
  result = read (f, &read_cmd, file_size);

  free (core_img);

  if (result != 0) {
    printf ("CvP encountered an error! Try re-running reprogram with full SOF.\n");
    return 0;
  } else {
    printf ("Verifying device functionality right after CvP...\n");
    if (test_auto_discovery_read (f) && test_global_mem(f)) {
      printf ("CvP worked!\n");
      return 1;
    } else {
      printf ("The device doesn't work after CvP!\n");
      return 0;
    }
  }
}

/* Read constant data in FPGA ROM. Requires PCIe device on FPGA to work
 * and the whole FPGA have a valid OpenCL image */
#define ACL_CONFIGURATION_STORAGE_BAR 2
#define ACL_CONFIGURATION_STORAGE_SIZE 4096
#define ACL_CONFIGURATION_STORAGE_OFFSET (void*)0x5000
int test_auto_discovery_read (ssize_t f) {
  int i;
  char config_str[16+1];
  unsigned char bData = 'a';
  struct aclpci_cmd read_cmd = { 
        .bar_id      = ACL_CONFIGURATION_STORAGE_BAR,
        .command     = ACLPCI_CMD_DEFAULT,
        .device_addr = 0, /* will be set later */
        .user_addr   = &bData
  };
     
  for (i = 0; i < 16; i++) {
     read_cmd.device_addr = ACL_CONFIGURATION_STORAGE_OFFSET + i;
     read (f, &read_cmd, sizeof(char));
     config_str[i] = bData;
     if (bData == 0) break;     
  }
  config_str[16] = '\0';
  long version_number = atol(config_str) ;
  /* CvP was first enabled in version 11. All previous versions have different periphery 
   * and using CvP with them will lead to a non-functioning device. */
  if (version_number >= 11 && version_number < 1000) {
    printf ("Auto-discovery read test passed\n");
    return 1;
  } else {
    printf ("The version number looks invalid!\n");
    printf ("First 16 bytes of Auto-Discovered String: %s\n", config_str );
    return 0;
  }
}


/* Check if can write and read back full width of DDR global memory.
 * Requires DDR controller on FPGA to function properly. */
int test_global_mem (ssize_t f) {

  ssize_t i, i_ddr;
  ssize_t DDR_WIDTH = 512 / 8; /* 512 bits = 64 bytes */
  ssize_t NUM_DDR_BANKS = 2;
  ssize_t DDR_BANK_SIZE = (1 << 30);
  ssize_t DDR_STRIP_SIZE = (1 << 10);
  int val = 0xdeadbeef;
  int read_val = 0;
  struct aclpci_cmd read_cmd = { 0, ACLPCI_CMD_DEFAULT, 0, &val };

  
  for (i_ddr = 0; i_ddr < NUM_DDR_BANKS; i_ddr++) {

    // 1 GB + 1 KB will be in 2nd DDR bank, for both swdimm and striped
    ssize_t addr_offset = i_ddr * DDR_BANK_SIZE + (i_ddr > 0 ? 1 : 0) * DDR_STRIP_SIZE;
    
    /* Write to first DDR_WIDTH bytes. This utilizes the full DDR interface */
    for (i = 0; i < DDR_WIDTH; i += sizeof(int)) {
      read_cmd.device_addr = (void*)(i + addr_offset);
      // printf ("Writing to device addr %p\n", read_cmd.device_addr);
      write (f, &read_cmd, sizeof(val));
    }

    /* Now read back what we wrote */
    read_cmd.user_addr = &read_val;  
    for (i = 0; i < DDR_WIDTH; i += sizeof(int)) {
      read_val = 0;
      read_cmd.device_addr = (void*)(i + addr_offset);
      // printf ("Reading from device addr %p\n", read_cmd.device_addr);
      read (f, &read_cmd, sizeof(val));
      if (read_val != val) {
        printf ("Wrote %d to DDR%zu of global memory %p, got back %d! FPGA can't do global memory operations.", 
                val, i_ddr, read_cmd.device_addr, read_val);
        return 0;
      }
    }
  }

  printf ("Global memory simple read/write test to both DDR banks passed\n");
  return 1;
}

