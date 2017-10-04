#include "legup_mem.h"
#include "legup_mem_shared.h"
#include "riffa_wrapper.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Initialize the initial free block. Its free list pointers create a circular linked list to itself
S_NODE free_ddr_blk = {DDR_BASE_ADDR, &free_ddr_blk, &free_ddr_blk, NULL, NULL, (FREE_HDR << 31) | DDR_HEAP_SIZE};
S_NODE free_onchip_blk = {ONCHIP_BASE_ADDR, &free_onchip_blk, &free_onchip_blk, NULL, NULL, (FREE_HDR << 31) | ONCHIP_HEAP_SIZE};

void *malloc_shared(size_t size, void *original_ptr, LEGUP_RAM_LOCATION ram_location)
{
  size = size<8?8:size;
  return malloc_shared_internal(size, ram_location);
}

void free_shared(void *ptr)
{
  free_shared_internal(ptr);
}

size_t base_addr(LEGUP_RAM_LOCATION ram_location)
{
  return (ram_location == LEGUP_RAM_LOCATION_ONCHIP) ? ONCHIP_BASE_ADDR:DDR_BASE_ADDR;
}

void memcpy_from_shared(void *dst, void *src, size_t num)
{
  
  size_t transferSize = num;
  void *transferBuffer = dst;

  if (num < 8) {
    transferSize = 8;
    transferBuffer = malloc(8);
  }

  pci_read_dma(transferBuffer, (int)transferSize, (int)(size_t)src);
  pci_dma_go();

  if (num < 8) {
    memcpy(dst, transferBuffer, num);
    free(transferBuffer);
  }
  
#ifdef DEBUG_SIM
  int debug_num = (int)num;
  long long *debug_dst = (long long *)dst;
  long long *debug_src = (long long *)src;

  // second argument is number of 64-bit values to read, where num is number of bytes to read
  printf("read 0x%x %zu\n", (int)debug_src, (num + sizeof(long long) - 1) / sizeof(long long));

  while (debug_num > 0) {
    long long debug_val = *debug_dst;
    // mask out undefined data if needed
    if (debug_num < sizeof(long long)) {
      debug_val &= -1LL >> 8 * (sizeof(long long) - debug_num);
    }
    printf("# Expected: 0x%016llx\n", debug_val);
    debug_dst++;
    debug_num -= sizeof(long long);
  }
#endif
}

void memcpy_to_shared(void *dst, void *src, size_t num)
{
#ifdef DEBUG_SIM
  int debug_num = (int)num;
  long long *debug_dst = (long long *)dst;
  long long *debug_src = (long long *)src;
  while (debug_num > 0) {
    long long debug_val = *debug_src;
    // mask out undefined data if needed
    if (debug_num < sizeof(long long)) {
      debug_val &= -1LL >> 8 * (sizeof(long long) - debug_num);
    }
    printf("write 0x%08x 0x%016llx\n", (int)debug_dst, debug_val);
    debug_dst++;
    debug_src++;
    debug_num -= sizeof(long long);
  }
#endif
  
  size_t transferSize = num;
  void *transferBuffer = src;

  if (num < 8) {
    transferSize = 8;
    transferBuffer = malloc(8);
    memcpy(transferBuffer, src, num);
  }

  pci_write_dma(transferBuffer, (int)transferSize, (int)(size_t)dst);//(int)(size_t)dst);
  pci_dma_go();

  if (num < 8) {
    free(transferBuffer);
  }
}
