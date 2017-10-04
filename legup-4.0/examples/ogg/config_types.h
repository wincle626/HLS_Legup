#ifndef __CONFIG_TYPES_H__
#define __CONFIG_TYPES_H__

#include "mm.h"
#include "lib.h"

/* make it easy on the folks that want to compile the libs with a
   different malloc than stdlib */
#define _ogg_malloc  (void *)mm_malloc
#define _ogg_calloc  (void *)mm_calloc
#define _ogg_realloc (void *)mm_realloc
#define _ogg_free    mm_free

#    include <stdint.h>

/* these are filled in by configure */
typedef int16_t ogg_int16_t;
typedef u_int16_t ogg_uint16_t;
typedef int32_t ogg_int32_t;
typedef u_int32_t ogg_uint32_t;
typedef int64_t ogg_int64_t;

#endif
