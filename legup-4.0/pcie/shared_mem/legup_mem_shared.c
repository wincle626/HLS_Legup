#include "legup_mem.h"
#include "legup_mem_shared.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <search.h>

extern S_NODE free_ddr_blk;
extern S_NODE free_onchip_blk;

#define SIZE(x) (size_t)((x)->hdr & 0x7fffffff)

/* Binary tree stuff to lookup used block from address */
void *root = NULL;

S_NODE *root_node_for_memory_address(size_t addr) {
  if (addr & ONCHIP_TAG_MASK) {
    return &free_onchip_blk;
  }
  return &free_ddr_blk;
}

int compare(const void *pa, const void *pb)
{
   if (((S_NODE *)pa)->addr < ((S_NODE *)pb)->addr)
     return -1;
   if (((S_NODE *)pa)->addr > ((S_NODE *)pb)->addr)
     return 1;
   return 0;
}

// print address of each node (debugging)
void action(const void *nodep, const VISIT which, const int depth)
{
  int *datap;

  switch (which) {
    case preorder:
    case endorder:
      break;
    case postorder:
    case leaf:
      datap = *(int **) nodep;
      printf("%x\n", *datap);
      break;
  }
}
/* End binary tree stuff */

// Allocate a free block and return it
void *use_free_block(S_NODE *finger)
{
  finger->hdr |= (USED_HDR << 31);
  if (finger->prev) {
    finger->prev->next = finger->next;
  }
  if (finger->next) {
    finger->next->prev = finger->prev;
  }

  // add address->used block to tree and cleanup
  void *val = tsearch(finger, &root, compare);
  if (val == NULL) {
    return NULL;
  }

  return (void *)(finger->addr);
}

void *malloc_shared_internal(size_t size, LEGUP_RAM_LOCATION ram_location)
{

  S_NODE *free_blk;

  if (ram_location == LEGUP_RAM_LOCATION_ONCHIP) {
    free_blk = &free_onchip_blk;
  }
  else if (ram_location == LEGUP_RAM_LOCATION_DDR2) {
    free_blk = &free_ddr_blk;
  }
  else {
    printf("Specified RAM location does not exist.\n");
    return 0;
  }

  // Round size up to nearest ALIGN bytes
  size = (size + ALIGN-1) &~(ALIGN-1);

  S_NODE *finger = free_blk;
  do {
    finger = finger->next;
    if (SIZE(finger) >= size) {
      break;
    }
  } while (finger != free_blk);

  // Check heap space
  if (SIZE(finger) < size) {
    return NULL;
  }

  // Transform free block into a used block
  if (SIZE(finger) == size && finger != free_blk) {
    return use_free_block(finger);
  }

  // Split free block
  S_NODE *node = (S_NODE *)malloc(sizeof(S_NODE));
  if (!node) {
    return NULL;
  }
  node->addr = finger->addr;
  node->hdr = (USED_HDR << 31) | size;
  node->prev = node->next = NULL;
  node->up = finger;
  node->down = finger->down;
  if (node->down) {
    node->down->up = node;
  }

  // Update finger
  finger->addr += size;
  finger->hdr = (FREE_HDR << 31) | (SIZE(finger) - size);
  finger->down = node;

  // add address->used block to tree and cleanup
  void *val = tsearch(node, &root, compare);
  if (val == NULL) {
    return NULL;
  }

  return (void *)(node->addr);
}

// Pass the up pointer to coalesce
void coalesce_up(S_NODE *finger)
{
  finger->addr = finger->down->addr;
  finger->hdr = SIZE(finger) + SIZE(finger->down);
  finger->down = finger->down->down;
  if (finger->down) {
    finger->down->up = finger;
  }
}

// Pass the down pointer to coalesce
void coalesce_down(S_NODE *finger)
{
  finger->hdr = SIZE(finger) + SIZE(finger->up);
  finger->up = finger->up->up;
  if (finger->up) {
    finger->up->down = finger;
  }
}

// Free a used block
void free_used_block(S_NODE *finger)
{

  S_NODE *free_blk = root_node_for_memory_address(finger->addr);

  // set block as free
  finger->hdr &= ~(USED_HDR << 31);
  // insert freed block to front
  finger->next = free_blk->next;
  finger->prev = free_blk;
  free_blk->next = finger;
  if (finger->next) {
    finger->next->prev = finger;
  }
}

// Perform coalescing on freed block
void free_coalesce(S_NODE *finger, S_NODE *temp_ptr)
{
  S_NODE *free_blk = root_node_for_memory_address(finger->addr);
  
  // can't free finger until the tree node is free (at the end)
  S_NODE *free_finger = (finger != free_blk) ? finger : NULL;
  bool coalesced = false;

  // coalesce with up
  if (finger->up && (finger->up->hdr >> 31) == FREE_HDR) {
    finger = finger->up;
    coalesce_up(finger);
    coalesced = true;
  }
  // coalesce with bottom
  if (finger->down && (finger->down->hdr >> 31) == FREE_HDR) {
    if (coalesced) {
      // let the upper block take over the lower block
      S_NODE *to_del = finger->down;
      if (to_del->prev) {
        to_del->prev->next = to_del->next;
      }
      if (to_del->next) {
        to_del->next->prev = to_del->prev;
      }
      coalesce_up(finger);
      free(to_del);
    } else {
      finger = finger->down;
      coalesce_down(finger);
      coalesced = true;
    }
  }
  if (!coalesced) {
    free_used_block(finger);

    // don't free finger since it is a free block
    free_finger = NULL;
  }

  // remove newly freed ptr from used tree
  tdelete(temp_ptr, &root, compare);
  free(free_finger);
}

void free_shared_internal(void *ptr)
{
  // No-op
  if (!ptr) {
    return;
  }

  // Used to find the used block, given an address
  S_NODE temp_node;

  size_t addr = (size_t)ptr;
  temp_node.addr = addr;
  S_NODE *temp_ptr = &temp_node;

  // Error check
  S_NODE **finger_ptr = tfind(temp_ptr, &root, compare);
  if (!finger_ptr || (*finger_ptr)->addr != addr) {
    fprintf(stdout, "Couldn't free pointer %zx\n", addr);
    exit(1);
  }

  // coalesce finger to free it
  S_NODE *finger = *finger_ptr;

  free_coalesce(finger, temp_ptr);
}
