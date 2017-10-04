#include <assert.h>

#include "legup_mem.h"
#include "legup_mem_shared.h"

// Test malloc alignment, upper limit
// Test free with no coalescing and both coalescing
void test_1(int *ptr)
{
  // First pointer should be allocated at the base_addr(LEGUP_RAM_LOCATION_DDR2)
  int *shared_ptr = malloc_shared(sizeof(int), ptr, LEGUP_RAM_LOCATION_DDR2);
  assert(shared_ptr);
  assert((size_t)shared_ptr == base_addr(LEGUP_RAM_LOCATION_DDR2));

  // Ensure the second pointer is aligned
  int *shared_ptr_2 = malloc_shared(sizeof(int), ptr, LEGUP_RAM_LOCATION_DDR2);
  assert(shared_ptr_2);
  assert((size_t)shared_ptr_2 == (base_addr(LEGUP_RAM_LOCATION_DDR2)+ALIGN));

  // Go over the shared memory heap size, should fail and return NULL
  int *shared_ptr_3 = malloc_shared(DDR_HEAP_SIZE, NULL, LEGUP_RAM_LOCATION_DDR2);
  assert(!shared_ptr_3);

  // Allocate some more memory
  shared_ptr_3 = malloc_shared(100*ALIGN, ptr, LEGUP_RAM_LOCATION_DDR2);
  assert(shared_ptr_3 && (size_t)shared_ptr_3 == (base_addr(LEGUP_RAM_LOCATION_DDR2)+2*ALIGN));

  free_shared(shared_ptr_2);
  shared_ptr_2 = NULL;

  // Can't fit into free block
  int *shared_ptr_4 = malloc_shared(2*ALIGN, ptr, LEGUP_RAM_LOCATION_DDR2);
  assert(shared_ptr_4);
  assert((size_t)shared_ptr_4 == (base_addr(LEGUP_RAM_LOCATION_DDR2)+102*ALIGN));

  // Can fit into free block
  shared_ptr_2 = malloc_shared(ALIGN, ptr, LEGUP_RAM_LOCATION_DDR2);
  assert(shared_ptr_2);
  assert((size_t)shared_ptr_2 == (base_addr(LEGUP_RAM_LOCATION_DDR2)+ALIGN));

  free_shared(NULL);
  // coalesce none
  free_shared(shared_ptr);
  // coalesce none
  free_shared(shared_ptr_3);
  // coalesce both
  free_shared(shared_ptr_4);
  // coalesce both
  free_shared(shared_ptr_2);
}

// Test free with more coalesce variations (up)
void test_2(int *ptr)
{
  // Re-allocate and free in a different order
  int *shared_ptr = malloc_shared(ALIGN, ptr, LEGUP_RAM_LOCATION_DDR2);
  assert(shared_ptr && (size_t)shared_ptr == base_addr(LEGUP_RAM_LOCATION_DDR2));

  int *shared_ptr_2 = malloc_shared(ALIGN, ptr, LEGUP_RAM_LOCATION_DDR2);
  assert(shared_ptr_2 && (size_t)shared_ptr_2 == base_addr(LEGUP_RAM_LOCATION_DDR2)+ALIGN);

  int *shared_ptr_3 = malloc_shared(ALIGN, ptr, LEGUP_RAM_LOCATION_DDR2);
  assert(shared_ptr_3 && (size_t)shared_ptr_3 == base_addr(LEGUP_RAM_LOCATION_DDR2)+2*ALIGN);

  int *shared_ptr_4 = malloc_shared(ALIGN, ptr, LEGUP_RAM_LOCATION_DDR2);
  assert(shared_ptr_4 && (size_t)shared_ptr_4 == base_addr(LEGUP_RAM_LOCATION_DDR2)+3*ALIGN);

  // coalesce none
  free_shared(shared_ptr_3);
  // coalesce up
  free_shared(shared_ptr_2);
  // coalesce both
  free_shared(shared_ptr);
  // coalesce both
  free_shared(shared_ptr_4);
}

// Test free with more coalesce variations (down)
void test_3(int *ptr)
{
  // Re-allocate and free in a different order
  int *shared_ptr = malloc_shared(ALIGN, ptr, LEGUP_RAM_LOCATION_DDR2);
  assert(shared_ptr && (size_t)shared_ptr == base_addr(LEGUP_RAM_LOCATION_DDR2));

  int *shared_ptr_2 = malloc_shared(ALIGN, ptr, LEGUP_RAM_LOCATION_DDR2);
  assert(shared_ptr_2 && (size_t)shared_ptr_2 == base_addr(LEGUP_RAM_LOCATION_DDR2)+ALIGN);

  // coalesce down
  free_shared(shared_ptr);
  // coalesce both
  free_shared(shared_ptr_2);

  // Ensure lowest block is free
  shared_ptr = malloc_shared(ALIGN, ptr, LEGUP_RAM_LOCATION_DDR2);
  assert(shared_ptr && (size_t)shared_ptr == base_addr(LEGUP_RAM_LOCATION_DDR2));

  free_shared(shared_ptr);
}

int main()
{
  int a;
  int *ptr = &a;

  test_1(ptr);
  test_2(ptr);
  test_3(ptr);

  return 0;
}
