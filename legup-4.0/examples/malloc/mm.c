/*
 * Explicit free list implementation with a first fit policy using a circular
 * linked list of free blocks.
 *
 * malloc may allocate a larger block to ensure later blocks fit easier.
 * The current implementation is to round up to the nearest power of 2, if the
 * size is at least 3/4 of that amount.
 *
 * Realloc is implemented using malloc and free an added optimization that
 * prevents the need to memcpy (and equivalent to freeing first before malloc).
 */
#include "mm.h"
#include "util.h"

/*************************************************************************
 * Basic Constants and Macros
 * Acquired from text Computer Systems - A Programmer's Perspective
 * Figure 10.45
 * You are not required to use these macros but may find
 * them helpful.
*************************************************************************/
#define WSIZE       4                /* word size (bytes) */
#define DSIZE       8                /* doubleword size (bytes) */
#define CHUNKSIZE   (1<<12)          /* initial heap size (bytes) */
#define OVERHEAD    8                /* overhead of header and footer (bytes) */
#define FREE_OVERHEAD	24           /* overhead of a free block (bytes) */

#define MAX(x,y) ((x) > (y)?(x) :(y))
#define MIN(x,y) ((x) < (y)?(x) :(y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a size_t at address p */
#define GET(p)          (*(size_t *)(p))
#define PUT(p,val)      (*(size_t *)(p) = (size_t)(val))

/* Read and write a word at address p */
#define GET_INT(p)      (*(unsigned *)(p))
#define PUT_INT(p, val) (*(unsigned *)(p) = (unsigned)(val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)     (GET_INT(p) & ~0x7)
#define GET_ALLOC(p)    (GET_INT(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)        ((char *)(bp) - WSIZE)
#define FTRP(bp)        ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

/* add address to ptr, return as void* */
#define ADD_PTR(p, a) ((void *)((char *)(p) + (a)))
#define PREV_PTR(p)   ((size_t *)((char *)(p)))
#define NEXT_PTR(p)   ((size_t *)((char *)(p) + DSIZE))

/* global pointer, which keeps track of the free_list head and tail */
#define free_list (mem_heap_lo())

/* do not round up to next power of two if greater than LARGE_SIZE */
#define LARGE_SIZE 2048

/* round up to next power of 2 for 32-bit integer from
http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2 */
unsigned round_up_two(unsigned v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

/* rounded allocation size for malloc blocks */
unsigned malloc_round(unsigned n)
{
  // do not round if greater than LARGE_SIZE
  if (n > LARGE_SIZE)
    return n;
  unsigned v = round_up_two(n);
  // only round up if size already 3/4 of the way there
  return (n * 4 > v * 3) ? v : n;
}

/* put header, footer and free_list pointers in a free block */
void free_ptr(void *ptr, void *prev, void *next, unsigned size)
{
    unsigned header = PACK(size, 0);
    // header
    PUT_INT(HDRP(ptr), header);
    // footer
    PUT_INT(FTRP(ptr), header);
    // prev and next
    PUT(PREV_PTR(ptr), prev);
    PUT(NEXT_PTR(ptr), next);
}

/* coalesce a pointer and free it */
void coalesce_free_ptr(void *ptr, unsigned size)
{
    // check next ptr
    if (!GET_ALLOC(HDRP(NEXT_BLKP(ptr)))) {
      unsigned nextSize = GET_SIZE(HDRP(NEXT_BLKP(ptr)));
      void *nextPtr = NEXT_BLKP(ptr);
      // check prev ptr
      if (!GET_ALLOC(HDRP(PREV_BLKP(ptr)))) {
        // coalesce with both adjacent blocks
        unsigned prevSize = GET_SIZE(HDRP(PREV_BLKP(ptr)));
        void *prevPtr = ADD_PTR(ptr, -prevSize);
        // prevPtr->next->prev = prevPtr->prev;
        PUT(PREV_PTR(GET(NEXT_PTR(prevPtr))), GET(PREV_PTR(prevPtr)));
        // prevPtr->prev->next = prevPtr->next;
        PUT(NEXT_PTR(GET(PREV_PTR(prevPtr))), GET(NEXT_PTR(prevPtr)));
        // nextPtr->next->prev = nextPtr->prev;
        PUT(PREV_PTR(GET(NEXT_PTR(nextPtr))), GET(PREV_PTR(nextPtr)));
        // nextPtr->prev->next = nextPtr->next;
        PUT(NEXT_PTR(GET(PREV_PTR(nextPtr))), GET(NEXT_PTR(nextPtr)));
        // create coalesced free block, stick block at end of free_list
        free_ptr(prevPtr, (void *)GET(PREV_PTR(free_list)), free_list,
            prevSize + size + nextSize);
        // tail->prev->next = prevPtr;
        PUT(NEXT_PTR(GET(PREV_PTR(free_list))), prevPtr);
        // tail->prev = prevPtr;
        PUT(PREV_PTR(free_list), prevPtr);
        return;
      }
      // coalesce with next block
      // nextPtr->prev->next = ptr;
      PUT(NEXT_PTR(GET(PREV_PTR(nextPtr))), ptr);
      // nextPtr->next->prev = ptr;
      PUT(PREV_PTR(GET(NEXT_PTR(nextPtr))), ptr);
      // create coalesced free block
      free_ptr(ptr, (void *)GET(PREV_PTR(nextPtr)),
          (void *)GET(NEXT_PTR(nextPtr)), size + nextSize);
      return;
    }
    if (!GET_ALLOC(HDRP(PREV_BLKP(ptr)))) {
      // coalesce with previous block
      unsigned prevSize = GET_SIZE(HDRP(PREV_BLKP(ptr)));
      void *prevPtr = ADD_PTR(ptr, -prevSize);
      // create coalesced free block
      free_ptr(prevPtr, (void *)GET(PREV_PTR(prevPtr)),
          (void *)GET(NEXT_PTR(prevPtr)), prevSize + size);
    } else {
      // don't coalesce
      // tail->prev->next = ptr;
      PUT(NEXT_PTR(GET(PREV_PTR(free_list))), ptr);
      // create free block
      free_ptr(ptr, (void *)GET(PREV_PTR(free_list)), free_list, size);
      // tail->prev = ptr;
      PUT(PREV_PTR(free_list), ptr);
    }
}

/* put header and footer for an allocated block */
void alloc_ptr(void *ptr, unsigned size)
{
    unsigned header = PACK(size, 1);
    PUT_INT(HDRP(ptr), header);
    PUT_INT(FTRP(ptr), header);
}

/*
 * mm_init - initialize the malloc package.
 *
 * Allocates free_list next and prev pointers as a circular linked list.
 * Creates an "allocated" footer to prevent coalescing at the start of the
 * heap.
 */
int mm_init(void)
{
    // for free_list head and tail
    mem_sbrk(2 * DSIZE);
    // set head and tail to point to free_list
    PUT(PREV_PTR(free_list), free_list);
    PUT(NEXT_PTR(free_list), free_list);
    // ensure there is space for an extra double word before the heap begins
    void *heap_start = mem_sbrk(DSIZE);
    // set allocation before the first block
    PUT_INT(heap_start, 1);
    return 0;
}

/*
 * fit_size - return minimum size to fit into a block.
 */
unsigned fit_size(unsigned size)
{
    return MAX(ALIGN(OVERHEAD + size), FREE_OVERHEAD);
}

/*
 * malloc_size - return rounded up size to allocate for malloc.
 */
unsigned malloc_size(unsigned size)
{
    return MAX(ALIGN(OVERHEAD + malloc_round(size)), FREE_OVERHEAD);
}

/**
 * realloc_size - return size to allocate for realloc.
 */
unsigned realloc_size(unsigned size, unsigned overhead)
{
    return MAX(ALIGN(overhead + size), FREE_OVERHEAD);
}

/*
 * mm_malloc - Allocate as an explicit free list.
 *
 * This is done using a first fit policy on the minimum size of the block.
 * The first fitting block may be split up into a smaller free block and an
 * allocated free block to fit the block requested by malloc. The free block
 * will precede the allocated block to prevent the need to move pointers.
 *
 * If the block does not fit, a new allocated block is created, which may
 * round the block size up to the nearest power of 2. An "allocated" block is
 * created at the end to prevent coalescing past the heap end.
 */
void *mm_malloc(size_t size)
{
    // use malloc_size or fit_size
    unsigned newsize = fit_size(size);
    // finger = free_list->next
    size_t *finger = (size_t *)GET(NEXT_PTR(free_list));
    size_t *p = (size_t *)free_list;
    unsigned fitSize = ~0;
    // iterate linked list
    while (finger != free_list) {
      unsigned blkSize = GET_SIZE(HDRP(finger));
      // look for best fit, exit early if perfect fit
      if (blkSize == newsize) {
        p = finger;
        break;
      }
      // remember best fit
      if (blkSize > newsize && blkSize < fitSize) {
        p = finger;
        fitSize = blkSize;
      }
      // finger = finger->next
      finger = (size_t *)GET(NEXT_PTR(finger));
    }
    // if block doesn't fit in any free block, allocate more heap space
    if (p == free_list) {
      // round up with malloc_size
      newsize = malloc_size(size);
      // if block before end of heap is a free block, coalesce
      void *prevPtr = PREV_BLKP(ADD_PTR(mem_heap_hi(), 1));
      if (prevPtr < mem_heap_hi() && !GET_ALLOC(HDRP(prevPtr))) {
        // calculate new size to sbrk
        unsigned allocSize = newsize - GET_SIZE(HDRP(prevPtr));
        p = mem_sbrk(allocSize);
        if ((int)(p) == -1)
          return NULL;

        // prevPtr->prev->next = prevPtr->next;
        PUT(NEXT_PTR(GET(PREV_PTR(prevPtr))), GET(NEXT_PTR(prevPtr)));
        // prevPtr->next->prev = prevPtr->prev;
        PUT(PREV_PTR(GET(NEXT_PTR(prevPtr))), GET(PREV_PTR(prevPtr)));
        // allocate prevPtr to new size
        alloc_ptr(prevPtr, newsize);

        // set allocation before the last block
        PUT_INT(HDRP(NEXT_BLKP(prevPtr)), 1);

        return prevPtr;
      }
      p = mem_sbrk(newsize);
      if ((int)(p) == -1)
        return NULL;

      alloc_ptr(p, newsize);

      // set allocation before the last block
      PUT_INT(HDRP(NEXT_BLKP(p)), 1);
      
      return p;
    }
    unsigned freeSize = GET_SIZE(HDRP(p));
    if (freeSize - newsize >= FREE_OVERHEAD) {
      // split free block into free and allocated blocks

      unsigned header = freeSize - newsize;
      // header
      PUT_INT(HDRP(p), header);
      // footer
      PUT_INT(FTRP(p), header);
      p = ADD_PTR(p, freeSize - newsize);
    } else {
      // allocate the whole free block
      newsize = freeSize;
      // p->prev->next = p->next
      PUT(NEXT_PTR(GET(PREV_PTR(p))), GET(NEXT_PTR(p)));
      // p->next->prev = p->prev
      PUT(PREV_PTR(GET(NEXT_PTR(p))), GET(PREV_PTR(p)));
    }
    // sets header and footer with allocate bit set 
    alloc_ptr(p, newsize);

    return p;
}

/*
 * mm_free - Coalesce and free pointers.
 *
 * Checks the previous and next blocks so that they may be coalesced.
 * Blocks are freed by unsetting the header and footer allocate bits.
 * free_list pointers are always modified to add the newly freed block.
 */
void mm_free(void *ptr)
{
    coalesce_free_ptr(ptr, GET_SIZE(HDRP(ptr)));
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free,
 *              but with one added optimization.
 *
 * The optimization is that if the block is already at the end of the heap,
 * sbrk only the extra required size and update the header and footer.
 * No memcpy call is required and return the pointer that was passed in.
 */
void *mm_realloc(void *ptr, size_t size)
{
    unsigned blkSize = GET_SIZE(HDRP(ptr));
    if (realloc_size(size, 0) < blkSize) {
      return ptr;
    }

    // coalesce with sbrk
    if (NEXT_BLKP(ptr) == ADD_PTR(mem_heap_hi(), 1)) {
      unsigned allocSize = realloc_size(size, OVERHEAD) - blkSize;
      mem_sbrk(allocSize);
      alloc_ptr(ptr, realloc_size(size, OVERHEAD));

      // set allocation before the last block
      PUT_INT(HDRP(NEXT_BLKP(ptr)), 1);

      return ptr;
    }
/*
    // allocate coalesce with next free block

    // allocate coalesce with prev free block
*/
    void *newptr = mm_malloc((unsigned)size);
    if (newptr == NULL)
      return NULL;

    unsigned cpysize = MIN(blkSize - OVERHEAD, (unsigned)size);
    memcpy_8(newptr, ptr, cpysize);
    mm_free(ptr);

    return newptr;
}
