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

/* Independent tester of Altera OpenCL De4 board + driver without
 * the host run-time library. */

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
#include "pcie_linux_driver_exports.h"

typedef union {
  unsigned long long x;
  float r[2];
} u_res;

  /* Location of the auto-discover configuration storage information */
  #define ACL_CONFIGURATION_STORAGE_BAR 2
  #define ACL_CONFIGURATION_STORAGE_SIZE 4096
  #define ACL_CONFIGURATION_STORAGE_OFFSET (void*)0x5000
#define DEEP_DEBUG(x)
  

unsigned char read_uchar (ssize_t dev_id, int bar_id, void *dev_addr) {
  unsigned char val;
  struct aclpci_cmd read_cmd = { bar_id, ACLPCI_CMD_DEFAULT, dev_addr, &val };
  read (dev_id, &read_cmd, sizeof(val));
  return val;
}

unsigned short read_ushort (ssize_t dev_id, int bar_id, void *dev_addr) {
  unsigned short val;
  struct aclpci_cmd read_cmd = { bar_id, ACLPCI_CMD_DEFAULT, dev_addr, &val };
  read (dev_id, &read_cmd, sizeof(val));
  return val;
}

unsigned int read_uint (ssize_t dev_id, int bar_id, void *dev_addr) {
  unsigned int val;
  struct aclpci_cmd read_cmd = { bar_id, ACLPCI_CMD_DEFAULT, dev_addr, &val };
  read (dev_id, &read_cmd, sizeof(val));
  DEEP_DEBUG(printf ("-- Read 32bits %x from bar %d, addr %p\n", val, bar_id, dev_addr)); 
  return val;
}

unsigned long read_ulong (ssize_t dev_id, int bar_id, void *dev_addr) {
  unsigned long long val;
  struct aclpci_cmd read_cmd = { bar_id, ACLPCI_CMD_DEFAULT, dev_addr, &val };
  read (dev_id, &read_cmd, sizeof(val));
  DEEP_DEBUG(printf ("-- Read 64bits %lx from bar %d, addr %p\n", val, bar_id, dev_addr)); 
  return val;
}

size_t read_mem (ssize_t dev_id, int bar_id, 
                 void *dev_addr, void *dst_addr, size_t count) {

  struct aclpci_cmd read_cmd = { bar_id, ACLPCI_CMD_DEFAULT, dev_addr, dst_addr };
  size_t ret = read (dev_id, &read_cmd, count);
  return ret;
}


void write_uchar (ssize_t dev_id, int bar_id, void *dev_addr, unsigned char val) {
  struct aclpci_cmd read_cmd = { bar_id, ACLPCI_CMD_DEFAULT, dev_addr, &val };
  write (dev_id, &read_cmd, sizeof(val));
}

void write_ushort (ssize_t dev_id, int bar_id, void *dev_addr, unsigned short val) {
  struct aclpci_cmd read_cmd = { bar_id, ACLPCI_CMD_DEFAULT, dev_addr, &val };
  write (dev_id, &read_cmd, sizeof(val));
}

void write_uint (ssize_t dev_id, int bar_id, void *dev_addr, unsigned int val) {
  struct aclpci_cmd read_cmd = { bar_id, ACLPCI_CMD_DEFAULT, dev_addr, &val };
  write (dev_id, &read_cmd, sizeof(val));
  DEEP_DEBUG(printf ("-- Wrote 32bits %x to bar %d, addr %p\n", val, bar_id, dev_addr));
}

void write_ulong (ssize_t dev_id, int bar_id, void *dev_addr, unsigned long long val) {
  struct aclpci_cmd read_cmd = { bar_id, ACLPCI_CMD_DEFAULT, dev_addr, &val };
  write (dev_id, &read_cmd, sizeof(val));
  DEEP_DEBUG(printf ("-- Wrote 64bits %lx to bar %d, addr %p\n", val, bar_id, dev_addr));
}

size_t write_mem (ssize_t dev_id, int bar_id, 
                  void *dev_addr, void *src_addr, size_t count) {

  struct aclpci_cmd read_cmd = { bar_id, ACLPCI_CMD_DEFAULT, dev_addr, src_addr };
  size_t ret = write (dev_id, &read_cmd, count);
  return ret;
}

int test_global_mem (ssize_t f) {

  int val = 18987983;
  int read_val = 0;
  struct aclpci_cmd read_cmd = { 0, ACLPCI_CMD_DEFAULT, 0, &val };
  write (f, &read_cmd, sizeof(val));

  read_cmd.user_addr = &read_val;
  read (f, &read_cmd, sizeof(val));
  if (read_val != val) {
    printf ("Wrote %d to DDR0 of global memory, got back %d! FPGA can't do global memory operations.", val, read_val);
    return 0;
  }

  // 1 GB + 1 KB will be in 2nd DDR bank, for both swdimm and striped
  read_val = 0;
  read_cmd.device_addr = (void*)( (1 << 30) + (1 << 10) );
  read_cmd.user_addr = &val;

  write (f, &read_cmd, sizeof(val));
  read_cmd.user_addr = &read_val;
  read (f, &read_cmd, sizeof(val));
  if (read_val != val) {
    printf ("Wrote %d to DDR1 of global memory, got back %d! FPGA can't do global memory operations.", val, read_val);
    return 0;
  }

  printf ("Global memory simple read/write test to both DDR banks passed\n");
  return 1;
}


int dma_is_idle(ssize_t f)
{
   unsigned int result = 0;
   struct aclpci_cmd driver_cmd;
   driver_cmd.bar_id      = ACLPCI_CMD_BAR;
   driver_cmd.command     = ACLPCI_CMD_GET_DMA_IDLE_STATUS;
   driver_cmd.device_addr = NULL;
   driver_cmd.user_addr   = &result;
   read (f, &driver_cmd, sizeof(result));
   
   return (result != 0);
}

void dma_update(ssize_t f)
{
   struct aclpci_cmd driver_cmd;
   driver_cmd.bar_id      = ACLPCI_CMD_BAR;
   driver_cmd.command     = ACLPCI_CMD_DMA_UPDATE;
   driver_cmd.device_addr = NULL;
   driver_cmd.user_addr   = NULL;
   read (f, &driver_cmd, 0);
   
}

int get_device_id (ssize_t f)
{
   unsigned int result = 0;
   struct aclpci_cmd driver_cmd;
   driver_cmd.bar_id      = ACLPCI_CMD_BAR;
   driver_cmd.command     = ACLPCI_CMD_GET_DEVICE_ID;
   driver_cmd.device_addr = NULL;
   driver_cmd.user_addr   = &result;
   read (f, &driver_cmd, sizeof(result));
   
   return result;
}

int get_vendor_id (ssize_t f)
{
   unsigned int result = 0;
   struct aclpci_cmd driver_cmd;
   driver_cmd.bar_id      = ACLPCI_CMD_BAR;
   driver_cmd.command     = ACLPCI_CMD_GET_VENDOR_ID;
   driver_cmd.device_addr = NULL;
   driver_cmd.user_addr   = &result;
   read (f, &driver_cmd, sizeof(result));
   
   return result;
}


/* Test writing single values of various types to board's global memory.
 * The test passes if we read back what we wrote. */
void test_small_writes  (ssize_t f) {

  unsigned char     uc;
  unsigned short    us;
  unsigned int      ui;
  unsigned long int uli;

  write_uchar     (f, 0, 0, 19);
  uc = read_uchar (f, 0, 0);
  assert (uc == 19);
  
  write_ushort    (f, 0, 0, 13);
  us = read_ushort(f, 0, 0);
  assert (us == 13);

  write_ulong      (f, 0, 0, 0x3037383633352030);
  uli = read_ulong (f, 0, 0);
  assert (uli == 0x3037383633352030);
  
  write_uint     (f, 0, 0, 18987983);
  ui = read_uint (f, 0, 0);
  assert (ui == 18987983);
  
  printf ("test_small_writes PASSED\n");
}

void test_global_mem_write(ssize_t f) {
  unsigned char     uc;
  write_uchar     (f, 0, 0, 19);
  uc = read_uchar (f, 0, 0);
  if (uc != 19) {
    printf ("Write to global memory did not work!\n");
    printf ("If the board has removable DIMMs, make sure they are inserted fully and are good\n");
  }
}

/* Test reading and writing long chunks of data */
void test_large_read_write (ssize_t f) {

  int j, num_write_runs = 5;
  size_t buf_size = 80 * 1024 * 1024;
  char *buf1 = NULL;
  char *buf2 = NULL;
  const size_t ALIGNMENT = 4096;
  posix_memalign ((void**)&buf1, ALIGNMENT, buf_size);
  posix_memalign ((void**)&buf2, ALIGNMENT, buf_size);
  
  if (buf1 == NULL || buf2 == NULL) {
    printf ("Couldn't allocate memory! FAILED\n");
  }
  
  /* Some "real" data to fill memory with. */
  FILE *data_file = fopen ("tests/ulysses.txt", "r");
  if (data_file == NULL) {
    printf ("Couldn't open data file! FAILED\n");
  }

  /* read data file in chunks of 4 KB */
  size_t read_step = 1024 * 4;
  size_t num_read = 0;
  size_t incr = 0, file_size = 0;
  while ( (num_read = fread (buf1 + incr, sizeof(char), read_step, data_file)) ) {
    incr += num_read;
    if (num_read < read_step) {
      /* Done reading */
      break;
    }
  }
  /* Copy the file content until fill the buffer */
  file_size = incr;
  while (incr < (buf_size - file_size)) {
    memcpy (buf1 + incr, buf1, file_size);
    incr += file_size;
  }
  assert (incr < buf_size);
  incr = incr - (incr % ALIGNMENT);
  printf ("Useful data size for large read/write test is %zu bytes\n", incr);
  
  clock_t start = clock();
  clock_t e1, e2;
  
  /* Write to different locations on each run just to avoid
   * any kind of caching on PCIe block. */
  for (j = 0; j < num_write_runs; j++) {
    write_mem (f, 0, (void*)(incr * j), buf1, incr);
    while (!dma_is_idle(f))
      dma_update(f);
  }
  e1 = clock();

  read_mem (f, 0, 0, buf2, incr);
  while (!dma_is_idle(f))
      dma_update(f);
  e2 = clock();
  
  double t1 = ((double)e1 - start) / CLOCKS_PER_SEC;
  double t2 = ((double)e2 - e1) / CLOCKS_PER_SEC;
  int mb = 1024 * 1024;
  printf ("Writing %zu bytes took %.3f seconds (%.3f MB / sec)\n", 
        incr, t1, incr / mb / t1 * num_write_runs );
  printf ("Reading %zu bytes took %.3f seconds (%.3f MB / sec)\n", 
        incr, t2, incr / mb / t2 );
  
  /* Make sure what we read back is the same as what we wrote */
  assert (memcmp (buf1, buf2, incr) == 0);
  printf ("test_large_read_write PASSED\n");
  
  free (buf1);
  free (buf2);
  return;
}


void test_dma_writes(ssize_t f) {

  int PAGE_SIZE = 4*1024;
  int BUF_SIZE = PAGE_SIZE;
  char *data = 0;
  char *ret_data = 0;
  
  posix_memalign ((void*)&data, PAGE_SIZE, BUF_SIZE);
  posix_memalign ((void*)&ret_data, PAGE_SIZE, BUF_SIZE);
  
  memset (data, 1234, BUF_SIZE/4);
  sprintf (data, "Hello from DE4!\n");
  data[15] = '\0';
  fprintf (stderr, "First small part of what we're writing: %s\n", data);
  // fprintf (stderr, "Writing %d bytes to %p from %p\n", BUF_SIZE, 0, data);

  write_mem (f, 0, (void*)(0), data, BUF_SIZE);
  while (!dma_is_idle(f))
   dma_update(f);
  
  // fprintf (stderr, "Reading back %d bytes from %p to %p\n", BUF_SIZE, 0, ret_data);
  
  read_mem (f, 0, (void*)(0), ret_data, BUF_SIZE);
  while (!dma_is_idle(f))
   dma_update(f);
   
  fprintf (stderr, "First small part of data we got back: %s\n", ret_data);
  
  
  assert (memcmp(data, ret_data, BUF_SIZE) == 0);
  fprintf (stderr, "DMA write passed!\n");
  
  
  
  return;
}


void test_auto_discovery_read (ssize_t f) {

  int i;
  char config_str[ACL_CONFIGURATION_STORAGE_SIZE+1];
  char buf2[ACL_CONFIGURATION_STORAGE_SIZE+1];
  unsigned char bData = 'a';
  struct aclpci_cmd read_cmd = { 
        .bar_id      = ACL_CONFIGURATION_STORAGE_BAR,
        .command     = ACLPCI_CMD_DEFAULT,
        .device_addr = 0, /* will be set later */
        .user_addr   = &bData
  };
     
  for (i = 0; i < ACL_CONFIGURATION_STORAGE_SIZE; i++) {
     
     read_cmd.device_addr = ACL_CONFIGURATION_STORAGE_OFFSET + i;
     read (f, &read_cmd, sizeof(char));
     config_str[i] = bData;
     if (bData == 0) break;
  }
  config_str[ACL_CONFIGURATION_STORAGE_SIZE] = '\0';
  printf ("PCIe Auto-Discovered Param String (small reads): %s\n", config_str );
  
  #if 1
  /* Now try reading it in one shot and make sure we get the same answer. */
  read_mem (f,
            ACL_CONFIGURATION_STORAGE_BAR, 
            ACL_CONFIGURATION_STORAGE_OFFSET,
            buf2, 
            ACL_CONFIGURATION_STORAGE_SIZE);
  buf2[ACL_CONFIGURATION_STORAGE_SIZE] = '\0';
  // printf ("PCIe Auto-Discovered Param String (large read): %s\n", buf2 );

  assert (strcmp (config_str, buf2) == 0);
  #endif
  printf ("PCIe Auto-Discovery reading PASSED\n");
}


void test_reprogram(ssize_t f) {
  struct aclpci_cmd read_cmd = { ACLPCI_CMD_BAR, ACLPCI_CMD_SAVE_PCI_CONTROL_REGS, NULL, NULL };
  
  printf ("Testing reprogramming of the device\n");
  // Reprogram = save PCI state, load SOF, restore state
  ssize_t res = read (f, &read_cmd, 0);
  printf ("save_pci_state returned %zu\n", res);
  system ("quartus_pgm -c USB-Blaster -m jtag -o \"P;/data/ddenisen/opencl/p4/regtest/designs/fdtd3d/host/de4.sof\"");
  
  read_cmd.command = ACLPCI_CMD_LOAD_PCI_CONTROL_REGS;
  res = read (f, &read_cmd, 0);
  printf ("restore_pci_state returned %zu\n", res);
}

unsigned char *acl_loadFileIntoMemory (const char *in_file, size_t *file_size_out) {

  FILE *f = NULL;
  unsigned char *buf;
  size_t file_size;
  
  // When reading as binary file, no new-line translation is done.
  f = fopen (in_file, "rb");
  if (f == NULL) {
    fprintf (stderr, "Couldn't open file %s for reading\n", in_file);
    return NULL;
  }
  
  // get file size
  fseek (f, 0, SEEK_END);
  file_size = ftell (f);
  rewind (f);
  
  // slurp the whole file into allocated buf
  buf = (unsigned char*) malloc (sizeof(char) * file_size);
  *file_size_out = fread (buf, sizeof(char), file_size, f);
  fclose (f);
  
  if (*file_size_out != file_size) {
    fprintf (stderr, "Error reading %s. Read only %zu out of %zu bytes\n", 
                     in_file, *file_size_out, file_size);
    return NULL;
  }
  return buf;
}

void test_cvp (ssize_t f) {

  int result;
  int *core_img = NULL;
  size_t file_size;
  struct aclpci_cmd read_cmd = { 
        ACLPCI_CMD_BAR, 
        ACLPCI_CMD_DO_CVP, 
        NULL, 
        NULL };

  core_img =  (int*)acl_loadFileIntoMemory ("output_file.core.rbf", &file_size);
  if (core_img == NULL) {
    return;
  }
  /* file_size is in bytes */
  assert (file_size % 4 == 0);
  file_size = file_size/4;
  
  read_cmd.user_addr = core_img;
  printf ("file_size is %zu\n", file_size);
  printf ("Testing CvP reprogramming of the device...\n");
  result = read (f, &read_cmd, file_size);
  
  free (core_img);
  
  if (result != 0) {
    printf ("CvP encountered an error! Check 'dmesg' for more details\n");
  } else {
    printf ("CvP worked!\n");
  }
}


void print_pid() {
   char buf[1024];
   ssize_t num_read = readlink ("/proc/self", buf, 1024);
   buf[num_read] = '\0';
   printf ("PID of this process: %s\n", buf);
}

void print_help() {
    printf("---- Control --------------------\n");
    printf("  q) Quit\n");
    printf("  i) Device info\n");
    printf("  h) Print this message\n");
    printf("---- Tests ----------------------\n");
    printf("  1) Test auto-discovery\n");
    printf("  2) Test small writes\n");
    printf("  3) Test DMA\n");
}

int run_test(ssize_t f, char c) {
  int exit = 0;
  switch(c) {
    case 'q':
    case 'Q':
      exit = 1;
      break;
    case 'i':
    case 'I':
      printf ("Vendor id = 0x%x, Device id = 0x%x\n", 
              get_vendor_id(f), get_device_id(f));
      print_pid();
      break;
    case 'h':
    case 'H':
      print_help();
      break;
    case '1':
      test_auto_discovery_read(f);
      break;
    case '2':
      test_global_mem_write (f);
      break;
    case '3':
      test_dma_writes(f);
      break;
    default:
      printf("Invalid selection (%c).\n", c);
      exit = 1;
      break;
  }
  return exit;
}

int main() {
  ssize_t f = open ("/dev/de4", O_RDWR);
  if (f == -1) {
    printf ("Couldn't open the device. Did you load the driver?\n");
    return 0;
  } else {
    printf ("Opened the device: file handle #%zu!\n", f);
  }

  printf ("Vendor id = 0x%x, Device id = 0x%x\n", 
          get_vendor_id(f), get_device_id(f));
    
  print_pid();
  
  test_global_mem_write(f);
  
  int i;
  /* Change upper limit to a larger number (e.g. 1000) for stress testing */
  for (i = 0; i < 1; i++) {
    printf ("-- %d --\n", i);
     
   test_auto_discovery_read (f);
   test_small_writes (f);
   test_global_mem(f);
   // test_large_read_write (f);
   // test_reprogram(f);
   test_dma_writes(f);
   // test_cvp(f);
  }

  test_global_mem_write(f);
 
  printf ("Done testing!\n");
  
  close (f);
  return 0;
}
