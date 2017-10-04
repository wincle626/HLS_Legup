#ifndef _LEGUP_MEM_SHARED_H
#define _LEGUP_MEM_SHARED_H

#include <stddef.h>
#include "mem_constants_and_types.h"

void *malloc_shared_internal(size_t size, LEGUP_RAM_LOCATION ram_location);
void free_shared_internal(void *ptr);

size_t base_addr(LEGUP_RAM_LOCATION ram_location);

#endif
