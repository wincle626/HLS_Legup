/*
 * Copyright (c) 2013, Altera Corporation.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  - Redistributions of source code must retain the above copyright notice,
 *  this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright 
 * notice, this list of conditions and the following disclaimer in the 
 * documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

/* Queue of fixed size.
 * Uncomment below to run in user-space unit-test mode.
 * Otherwise, will compile in kernel mode. */
// #define UNIT_TEST_MODE

#include "aclpci_queue.h"

#ifdef UNIT_TEST_MODE
#include <stdlib.h> // for calloc
#include <stdio.h>  // for printf
#include <string.h> // for memcpy
#else
#include "aclpci.h"
#endif


void queue_init (struct queue *q, unsigned int elem_size, unsigned int size) {
  // printk ("queue_init %p, elem_size = %u, size = %u\n", q, elem_size, size);
  if (q == 0) { return; }
  #ifdef UNIT_TEST_MODE
    q->buffer = calloc (elem_size, size);
  #else
    q->buffer = kzalloc (elem_size * size, GFP_KERNEL);
  #endif
  if (q->buffer == 0) {
    printk ("Couldn't allocate queue buffer!\n");
    return;
  }
  q->size = size;
  q->elem_size = elem_size;
  q->count = 0;
  q->out = 0;
}


void queue_fini (struct queue *q) {
  // printk ("queue_init %p\n", q);
  if (q == 0) { return; }
  #ifdef UNIT_TEST_MODE
    free (q->buffer);
  #else
    kfree (q->buffer);
  #endif
  q->buffer = NULL;
  q->size = 0;
  q->elem_size = 0;
  q->count = 0;
  q->out = 0;
}


unsigned int queue_size (struct queue *q) {
  return q->count;
}

int queue_empty(struct queue *q) {
  return (q->count == 0);
}

/* localize ugly casts */
void *queue_addr (struct queue *q, unsigned int offset) {
  unsigned long buffer_loc = (unsigned long)q->buffer + offset * q->elem_size;
  return (void*)buffer_loc;
}

/* When working with the circular buffer, values can wrap around
 * at most once. So instead of doing val % size, can do a simple comparison */
unsigned int fast_mod (unsigned int val, unsigned int size) {
  if (val >= size)
    return val - size;
  else
    return val;
}

void queue_push (struct queue *q, void *e) {
  unsigned int loc;
  if (q->count == q->size) {
    /* queue is full! */
    return;
  }
  loc = fast_mod ( (q->out + q->count), q->size );
  memcpy (queue_addr(q, loc), e, q->elem_size);
  q->count++;
}

void queue_pop (struct queue *q) {
  if (q->count == 0) {
    return;
  }
  q->count--;
  q->out = fast_mod ( (q->out + 1), q->size );
}

void *queue_front (struct queue *q) {
  if (q->count == 0) {
    return NULL;
  }
  return queue_addr (q, q->out);
}

void *queue_back (struct queue *q) {
  if (q->count == 0) {
    return NULL;
  }
  return queue_addr (q, fast_mod( (q->out + q->count - 1), q->size ) );
}


/* Unit tests. */
#ifdef UNIT_TEST_MODE
int main() {
  struct queue q;
  int i, j, k;
  queue_init (&q, sizeof(int), 5);
  i = 1; queue_push(&q, &i);
  i = 2; queue_push(&q, &i);
  i = 3; queue_push(&q, &i);
  j = *(int*)queue_front(&q); k = *(int*)queue_back(&q);
  printf ("%d, %d\n", j, k);
  
  queue_pop(&q);
  j = *(int*)queue_front(&q); k = *(int*)queue_back(&q);
  printf ("%d, %d\n", j, k);
  
  queue_pop(&q);
  queue_pop(&q);
  i = 11; queue_push(&q, &i);
  i = 12; queue_push(&q, &i);
  i = 13; queue_push(&q, &i);
  i = 14; queue_push(&q, &i);
  i = 15; queue_push(&q, &i);
  i = 16; queue_push(&q, &i);
  j = *(int*)queue_front(&q);
  k = *(int*)queue_back(&q);
  printf ("%d, %d\n", j, k);
  
  while (!queue_empty(&q)) {
    int s = *(int*)queue_front(&q); queue_pop(&q);
    printf ("%d\n", s);
  }
  
  return 0;
}
#endif
