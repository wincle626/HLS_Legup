#include "legup_mem.h"

/* Dummy implementation so that the C-code is equivalent to not having shared memory */

// Return the original_ptr, since there is no shared memory, ie the returned pointer points to the same memory as the original ptr
void *malloc_shared(size_t size, void *original_ptr, LEGUP_RAM_LOCATION ram_location)
{
  return original_ptr;
}

// No pointer to free
void free_shared(void *ptr) {}

// No copying required because dst and src should be the same
void memcpy_from_shared(void *dst, void *src, size_t num) {}
void memcpy_to_shared(void *dst, void *src, size_t num) {}
