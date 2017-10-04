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

#define memset_1_i64 legup_memset_1_i64
#define memset_2_i64 legup_memset_2_i64
#define memset_4_i64 legup_memset_4_i64
#define memset_8_i64 legup_memset_8_i64
#define memcpy_1_i64 legup_memcpy_1_i64
#define memcpy_2_i64 legup_memcpy_2_i64
#define memcpy_4_i64 legup_memcpy_4_i64
#define memcpy_4_i64 legup_memcpy_4_i64
#define memcpy_8_i64 legup_memcpy_8_i64
#define memmove_1_i64 legup_memmove_1_i64
#define memmove_2_i64 legup_memmove_2_i64
#define memmove_4_i64 legup_memmove_4_i64
#define memmove_8_i64 legup_memmove_8_i64

void legup_memset_1(uint8_t *m, uint8_t c, uint32_t n);
void legup_memset_2(uint8_t *m, uint8_t c, uint32_t n);
void legup_memset_4(uint8_t *m, uint8_t c, uint32_t n);
void legup_memset_8(uint8_t *m, uint8_t c, uint32_t n);
void legup_memcpy_1(uint8_t *d, const uint8_t *s, uint32_t n);
void legup_memcpy_2(uint8_t *d, const uint8_t *s, uint32_t n);
void legup_memcpy_4(uint8_t *d, const uint8_t *s, uint32_t n);
void legup_memcpy_8(uint8_t *d, const uint8_t *s, uint32_t n);
void legup_memmove_1(uint8_t *d, const uint8_t *s, uint32_t n);
void legup_memmove_2(uint8_t *d, const uint8_t *s, uint32_t n);
void legup_memmove_4(uint8_t *d, const uint8_t *s, uint32_t n);
void legup_memmove_8(uint8_t *d, const uint8_t *s, uint32_t n);

void legup_memset_1_i64(uint8_t *m, uint8_t c, uint64_t n);
void legup_memset_2_i64(uint8_t *m, uint8_t c, uint64_t n);
void legup_memset_4_i64(uint8_t *m, uint8_t c, uint64_t n);
void legup_memset_8_i64(uint8_t *m, uint8_t c, uint64_t n);
void legup_memcpy_1_i64(uint8_t *d, const uint8_t *s, uint64_t n);
void legup_memcpy_2_i64(uint8_t *d, const uint8_t *s, uint64_t n);
void legup_memcpy_4_i64(uint8_t *d, const uint8_t *s, uint64_t n);
void legup_memcpy_8_i64(uint8_t *d, const uint8_t *s, uint64_t n);
void legup_memmove_1_i64(uint8_t *d, const uint8_t *s, uint64_t n);
void legup_memmove_2_i64(uint8_t *d, const uint8_t *s, uint64_t n);
void legup_memmove_4_i64(uint8_t *d, const uint8_t *s, uint64_t n);
void legup_memmove_8_i64(uint8_t *d, const uint8_t *s, uint64_t n);

void __legup_label(char *l);

#endif
