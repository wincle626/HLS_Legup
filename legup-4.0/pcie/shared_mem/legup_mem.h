#ifndef _LEGUP_MEM_H
#define _LEGUP_MEM_H

#include <stddef.h>
#include "mem_constants_and_types.h"

void *malloc_shared(size_t size, void *original_ptr, LEGUP_RAM_LOCATION ram_location);
void free_shared(void *ptr);

void memcpy_from_shared(void *dst, void *src, size_t num);
void memcpy_to_shared(void *dst, void *src, size_t num);

#endif
