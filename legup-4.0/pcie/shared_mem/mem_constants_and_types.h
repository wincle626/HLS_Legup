#ifndef LEGUP_MEM_CONSTANTS_AND_TYPES_H
#define LEGUP_MEM_CONSTANTS_AND_TYPES_H

typedef enum LEGUP_RAM_LOCATION {
    LEGUP_RAM_LOCATION_ONCHIP = 0,
    LEGUP_RAM_LOCATION_DDR2 = 1
} LEGUP_RAM_LOCATION;

// Starting address in the shared memory space
#define ONCHIP_BASE_ADDR 0x20000000
#define DDR_BASE_ADDR 0x40000000

// Bit Masks for tag bit
#define ONCHIP_TAG_MASK 0x20000000
#define DDR_TAG_MASK 0x40000000

// Size of shared memory space in bytes
#ifndef ONCHIP_HEAP_SIZE
#define ONCHIP_HEAP_SIZE 786432
#endif
#ifndef DDR_HEAP_SIZE
#define DDR_HEAP_SIZE 1073741824
#endif

// Alignment in bytes
#define ALIGN 8

enum FREE_OR_USED_HDR {
  FREE_HDR = 0,
  USED_HDR = 1
};

struct s_node{
  size_t addr;
  struct s_node *prev, *next; /* next and prev free blocks, if applicable */
  struct s_node *up, *down; /* physically adjacent blocks for coalescing */
  unsigned hdr; /* (free or used) << 31 | size) */
};

typedef struct s_node S_NODE;

#endif
