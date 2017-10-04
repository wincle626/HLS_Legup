#ifndef _UTIL_H
#define _UTIL_H

#include <stddef.h>
#include <stdint.h>
#define SIZE (1 << 21)

void *mem_heap_lo();

void *mem_heap_hi();

void *mem_sbrk(int size);

void memcpy_8(uint64_t * d, const uint64_t * s, size_t n);

#endif
