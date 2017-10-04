#include "util.h"

struct mem {
  char storage[SIZE];
} heap;

char *ptr = heap.storage;

void *mem_heap_lo() {
  return heap.storage;
}

void *mem_heap_hi() {
  return (ptr - 1);
}

void *mem_sbrk(int size) {
  void *p = ptr;
  ptr += size;
  // should check for reaching max heap size
  return p;
}

void memcpy_8(uint64_t * d, const uint64_t * s, size_t n)
{
    uint64_t * dt = d;
    const uint64_t * st = s;
    n >>= 3;
    while (n--)
        *dt++ = *st++;
}
