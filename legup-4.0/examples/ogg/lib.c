// library functions, simplified

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include "mm.h"

void *memcpy(void * d, const void * s, size_t n)
{
    uint8_t * dt = (uint8_t *)d;
    const uint8_t * st = (uint8_t*)s;
    while (n--)
        *dt++ = *st++;
    return d;
}

void *memset(void * m, int c1, size_t n)
{
    // only use the first byte
    uint8_t c = c1 & 0xff;

    uint8_t *s = (uint8_t *) m;
    while (n--) {
        *s++ = c;
    }
    return m;
}

void *memmove(void * d, void * s, size_t n)
{
    if (d < s) {
        uint8_t * dt = (uint8_t *)d;
        const uint8_t * st = (uint8_t *)s;
        while (n--)
            *dt++ = *st++;
    } else if (d > s) {
        uint8_t * dt = (uint8_t *)(d + n);
        const uint8_t * st = (uint8_t *)(s + n);
        while (n--)
            *--dt = *--st;
    }
    return d;
}

void *memchr(void *ptr, int val, size_t num)
{
  while(num--)
  {
    if (*(unsigned char *)ptr == (unsigned char)val)
      return ptr;
    ptr++;
  }
  return NULL;
}

int memcmp(void *ptr1, void *ptr2, size_t num)
{
  while(num--)
  {
    unsigned char c1 = *(unsigned char *)ptr1;
    unsigned char c2 = *(unsigned char *)ptr2;
    if (c1 > c2)
      return 1;
    else if (c1 < c2)
      return -1;
    ptr1++;
    ptr2++;
  }
  return 0;
}

void *mm_calloc(size_t num, size_t size)
{
  long long * p = (long long *)mm_malloc(num * size);
  void * ptr = (void *)p;
  int i, n = (size + 7) >> 3;
  while(n--)
    *p++ = 0;
  return ptr;
}
