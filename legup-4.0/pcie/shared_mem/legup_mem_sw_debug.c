#include "legup_mem.h"
#include "legup_mem_shared.h"

#include <stdlib.h>
#include <string.h>

// If the HEAP_SIZE is greater than the MAX_HEAP_SIZE, then simulate the software memtest malloc with malloc() and free(), otherwise use the internal malloc and free, for the most accurate simulation
#define MAX_HEAP_SIZE (1 << 25) /* 32 MB */

// Initialize the initial free block. Its free list pointers create a circular linked list to itself
#if (ONCHIP_HEAP_SIZE < MAX_HEAP_SIZE)
char onchip_dummy_heap[ONCHIP_HEAP_SIZE];
char ddr_dummy_heap[DDR_HEAP_SIZE];
S_NODE free_onchip_blk = {(size_t)(&onchip_dummy_heap), &free_onchip_blk, &free_onchip_blk, NULL, NULL, (FREE_HDR << 31) | ONCHIP_HEAP_SIZE};
S_NODE free_ddr_blk = {(size_t)(&ddr_dummy_heap), &free_ddr_blk, &free_ddr_blk, NULL, NULL, (FREE_HDR << 31) | DDR_HEAP_SIZE};
#else
S_NODE free_blk = {};
#endif

/* Dummy implementation that actually mallocs, frees and copies, so that memory problems can be debugged with a software memory profiler */

void *malloc_shared(size_t size, void *original_ptr, LEGUP_RAM_LOCATION ram_location)
{
#if (ONCHIP_HEAP_SIZE < MAX_HEAP_SIZE)
	return malloc_shared_internal(size, ram_location);
#else
	return malloc(size);
#endif

}

void free_shared(void *ptr)
{
#if (HEAP_SIZE < MAX_HEAP_SIZE)
  free_shared_internal(ptr);
#else
  free(ptr);
#endif
}

size_t base_addr(LEGUP_RAM_LOCATION ram_location)
{
#if (ONCHIP_HEAP_SIZE < MAX_HEAP_SIZE)
  return (ram_location == LEGUP_RAM_LOCATION_ONCHIP) ? (size_t)(&onchip_dummy_heap):(size_t)(&ddr_dummy_heap);
#else
  return 0; // undefined
#endif
}

void memcpy_from_shared(void *dst, void *src, size_t num)
{
  memcpy(dst, src, num);
}

void memcpy_to_shared(void *dst, void *src, size_t num)
{
  memcpy(dst, src, num);
}
