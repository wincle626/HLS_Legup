#ifndef	INTERRUPT_H
#define	INTERRUPT_H

/*
 *  Copyright (C) 2006-2010  Anders Gavare.  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright  
 *     notice, this list of conditions and the following disclaimer in the 
 *     documentation and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE   
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 *  OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 *  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 *  SUCH DAMAGE.
 *
 *
 *  Definitions related to the Interrupt subsystem.
 */

#include "misc.h"

struct interrupt {
	/*  Functions used to assert and deassert the interrupt.  */
	void		(*interrupt_assert)(struct interrupt *);
	void		(*interrupt_deassert)(struct interrupt *);

	/*
	 *  The interrupt "line" number, or "pin" number, is an internal number
	 *  used by interrupt_assert() and interrupt_deassert(). It may
	 *  correspond to a physical interrupt number, or it may be ignored
	 *  completely.
	 *
	 *  This can either be a small integer corresponding to the interrupt
	 *  pin number, or an entire interrupt mask (for controllers with at
	 *  most 32 interrupts).
	 */
	uint32_t	line;

	/*
	 *  The extra pointer is a pointer to the (conceptual) interrupt
	 *  handler. For a CPU interrupt, this is a pointer to the CPU. For
	 *  e.g. a 8259 interrupt, it is a pointer to the dev_8259 which will
	 *  handle it.
	 */
	void		*extra;

	/*
	 *  Actual name of the interrupt. This is a complete "path", e.g.
	 *  "machine[0].cpu[1].irq[3].isa[14]". It is used for
	 *  connecting/disconnecting, and debug output.
	 */
	char		*name;
};


/*
 *  Macros to make code using the interrupt struct a bit more consise:
 */

#define	INTERRUPT_ASSERT(istruct)	(istruct).interrupt_assert(&(istruct))
#define	INTERRUPT_DEASSERT(istruct)	(istruct).interrupt_deassert(&(istruct))

#define	INTERRUPT_CONNECT(name,istruct)		{		\
	interrupt_handler_lookup(name, &(istruct));		\
	interrupt_connect(&(istruct), 0);			\
	}

#define	INTERRUPT_CONNECT_EXCLUSIVE(name,istruct)	{	\
	interrupt_handler_lookup(name, &(istruct));		\
	interrupt_connect(&(istruct), 1);			\
	}


/*
 *  Registration of interrupt handlers:
 *
 *  Each interrupt handler (i.e. CPUs, interrupt controllers, various bus
 *  controllers) should call interrupt_handler_register() to register itself.
 */

void interrupt_handler_register(struct interrupt *templ);
void interrupt_handler_remove(const char *name);
int interrupt_handler_lookup(const char *name, struct interrupt *templ);


/*
 *  Functions used to keep track of exclusive and non-exclusive users,
 *  respectively, of an interrupt.
 *
 *  If interrupt_connect() returns, it means it succeeded. On failure, the
 *  emulator exits. There may be up to 1 exclusive user and no non-exclusive
 *  users, or 0 exclusive users and any number of non-exclusive users.
 */

void interrupt_connect(struct interrupt *i, int exclusive);
void interrupt_disconnect(struct interrupt *i, int exclusive);


#endif	/*  INTERRUPT_H  */
