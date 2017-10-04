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

#ifndef ACLPCI_QUEUE_H
#define ACLPCI_QUEUE_H

/* FIFO for a fixed number of elements. Interface is the same as for
 * C++ STL queue<> adaptor.
 *
 * Implemented as a circular buffer in an array.
 * Could've used kfifo but its interface changes between kernel 
 * versions. So don't want to bother porting source code just for a fifo. */
 
struct queue {
  void *buffer;       /* Buffer to hold the data. Size is >= size * elem_size */
  unsigned int size;  /* number of elements */
  unsigned int elem_size; /* size of single element */
  unsigned int count;   /* number of valid entries */
  unsigned int out;     /* First valid entry */
};

void queue_init (struct queue *q, unsigned int elem_size, unsigned int size);
void queue_fini (struct queue *q);

unsigned int queue_size (struct queue *q);
int queue_empty(struct queue *q);

void queue_push (struct queue *q, void *e);
void queue_pop (struct queue *q);
void *queue_front (struct queue *q);
void *queue_back (struct queue *q);

#endif
