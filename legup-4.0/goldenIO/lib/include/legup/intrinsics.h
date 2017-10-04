#include <stdint.h>
#include <string.h>

#ifndef _LEGUP_INTRINSICS_H
#define _LEGUP_INTRINSICS_H

#define memset_1 legup_memset_1
#define memset_2 legup_memset_2
#define memset_4 legup_memset_4
#define memset_8 legup_memset_8
#define memcpy_1 legup_memcpy_1
#define memcpy_2 legup_memcpy_2
#define memcpy_4 legup_memcpy_4
#define memcpy_8 legup_memcpy_8
#define memmove_1 legup_memmove_1
#define memmove_2 legup_memmove_2
#define memmove_4 legup_memmove_4
#define memmove_8 legup_memmove_8

void legup_memset_1(uint8_t *m, uint32_t c, size_t n);

void legup_memset_2(uint16_t *m, uint32_t c, size_t n);

void legup_memset_4(uint32_t *m, uint32_t c, size_t n);

void legup_memset_8(uint64_t *m, uint32_t c, size_t n);

void legup_memcpy_1(uint8_t * d, const uint8_t * s, size_t n);

void legup_memcpy_2(uint16_t * d, const uint16_t * s, size_t n);

void legup_memcpy_4(uint32_t * d, const uint32_t * s, size_t n);

void legup_memcpy_8(uint64_t * d, const uint64_t * s, size_t n);

void legup_memmove_1(uint8_t * d, const uint8_t * s, size_t n);

void legup_memmove_2(uint16_t * d, const uint16_t * s, size_t n);

void legup_memmove_4(uint32_t * d, const uint32_t * s, size_t n);

void legup_memmove_8(uint64_t * d, const uint64_t * s, size_t n);

#endif
