/*
 *  Copyright (C) 2007-2009  Anders Gavare.  All rights reserved.
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
 *  COMMENT: Generic IRQ controller for the test machines
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "device.h"
#include "interrupt.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"

#include "testmachine/dev_irqc.h"


#define	N_IRQC_INTERRUPTS	32

struct irqc_data {
	struct interrupt irq;		/*  Connected to the CPU  */

	int		asserted;	/*  Current CPU irq assertion  */

	uint32_t	status;		/*  Interrupt Status register  */
	uint32_t	enabled;	/*  Interrupt Enable register  */
};


void irqc_interrupt_assert(struct interrupt *interrupt)
{
	struct irqc_data *d = (struct irqc_data *) interrupt->extra;
	d->status |= interrupt->line;

	if ((d->status & d->enabled) && !d->asserted) {
		INTERRUPT_ASSERT(d->irq);
		d->asserted = 1;
	}
}


void irqc_interrupt_deassert(struct interrupt *interrupt)
{
	struct irqc_data *d = (struct irqc_data *) interrupt->extra;
	d->status &= ~interrupt->line;

	if (!(d->status & d->enabled) && d->asserted) {
		INTERRUPT_DEASSERT(d->irq);
		d->asserted = 0;
	}
}


DEVICE_ACCESS(irqc)
{
	struct irqc_data *d = (struct irqc_data *) extra;
	uint64_t idata = 0, odata = 0;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	switch (relative_addr) {

	case DEV_IRQC_IRQ:
		/*  Status register:  */
		if (writeflag == MEM_READ) {
			odata = d->status;
		} else {
			fatal("[ irqc: WARNING! write to DEV_IRQC_IRQ ]\n");
		}
		break;

	case DEV_IRQC_MASK:
		/*  Mask interrupts by clearing Enable bits:  */
		if (writeflag == MEM_READ) {
			fatal("[ irqc: WARNING! read from DEV_IRQC_MASK ]\n");
		} else {
			int old_assert = d->status & d->enabled;
			int new_assert;

			d->enabled &= ~(1 << idata);

			new_assert = d->status & d->enabled;

			if (!old_assert && new_assert)
				INTERRUPT_ASSERT(d->irq);
			else if (old_assert && !new_assert)
				INTERRUPT_DEASSERT(d->irq);
		}
		break;

	case DEV_IRQC_UNMASK:
		/*  Unmask interrupts by setting Enable bits:  */
		if (writeflag == MEM_READ) {
			fatal("[ irqc: WARNING! read from DEV_IRQC_UNMASK ]\n");
		} else {
			int old_assert = d->status & d->enabled;
			int new_assert;

			d->enabled |= (1 << idata);

			new_assert = d->status & d->enabled;

			if (!old_assert && new_assert)
				INTERRUPT_ASSERT(d->irq);
			else if (old_assert && !new_assert)
				INTERRUPT_DEASSERT(d->irq);
		}
		break;

	default:if (writeflag == MEM_WRITE) {
			fatal("[ irqc: unimplemented write to "
			    "offset 0x%x: data=0x%x ]\n", (int)
			    relative_addr, (int)idata);
		} else {
			fatal("[ irqc: unimplemented read from "
			    "offset 0x%x ]\n", (int)relative_addr);
		}
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVINIT(irqc)
{
	struct irqc_data *d;
	char n[300];
	int i;

	CHECK_ALLOCATION(d = (struct irqc_data *) malloc(sizeof(struct irqc_data)));
	memset(d, 0, sizeof(struct irqc_data));

	/*  Connect to the CPU's interrupt pin:  */
	INTERRUPT_CONNECT(devinit->interrupt_path, d->irq);

	/*  Register the interrupts:  */
	for (i=0; i<N_IRQC_INTERRUPTS; i++) {
		struct interrupt templ;

		snprintf(n, sizeof(n), "%s.irqc.%i",
		    devinit->interrupt_path, i);

		memset(&templ, 0, sizeof(templ));
		templ.line = 1 << i;		/*  Note: line contains the
						    _mask_, not line number.  */
		templ.name = n;
		templ.extra = d;
		templ.interrupt_assert = irqc_interrupt_assert;
		templ.interrupt_deassert = irqc_interrupt_deassert;
		interrupt_handler_register(&templ);
	}

	/*  Default to all interrupts enabled:  */
	d->enabled = 0xffffffff;

	memory_device_register(devinit->machine->memory, devinit->name,
	    devinit->addr, DEV_IRQC_LENGTH, dev_irqc_access, d,
	    DM_DEFAULT, NULL);

	return 1;
}

