/*
 *  Copyright (C) 2003-2009  Anders Gavare.  All rights reserved.
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
 *  COMMENT: DEC KN02 mainbus (TurboChannel interrupt controller)
 *
 *  Used in DECstation type 2 ("3MAX").  See include/dec_kn02.h for more info.
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

#include "thirdparty/dec_kn02.h"


#define	DEV_KN02_LENGTH		0x1000


struct kn02_data {
	uint8_t		csr[sizeof(uint32_t)];

	/*  Dummy fill bytes, so dyntrans can be used:  */
	uint8_t		filler[DEV_KN02_LENGTH - sizeof(uint32_t)];

	struct interrupt irq;
	int		int_asserted;
};


/*
 *  kn02_interrupt_assert(), kn02_interrupt_deassert():
 *
 *  Called whenever a KN02 (TurboChannel) interrupt is asserted/deasserted.
 */
void kn02_interrupt_assert(struct interrupt *interrupt)
{
	struct kn02_data *d = (struct kn02_data *) interrupt->extra;
	d->csr[0] |= interrupt->line;
	if (d->csr[0] & d->csr[2] && !d->int_asserted) {
		d->int_asserted = 1;
		INTERRUPT_ASSERT(d->irq);
	}
}
void kn02_interrupt_deassert(struct interrupt *interrupt)
{
	struct kn02_data *d = (struct kn02_data *) interrupt->extra;
	d->csr[0] &= ~interrupt->line;
	if (!(d->csr[0] & d->csr[2]) && d->int_asserted) {
		d->int_asserted = 0;
		INTERRUPT_DEASSERT(d->irq);
	}
}


DEVICE_ACCESS(kn02)
{
	struct kn02_data *d = (struct kn02_data *) extra;
	uint64_t idata = 0, odata = 0;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	switch (relative_addr) {
	case 0:
		if (writeflag==MEM_READ) {
			odata = d->csr[0] + (d->csr[1] << 8) +
			    (d->csr[2] << 16) + (d->csr[3] << 24);

			/* debug("[ kn02: read from CSR: 0x%08x ]\n", odata); */
		} else {
			/*
			 *  Only bits 23..8 are considered writable. The
			 *  lowest 8 bits are actually writable, but don't
			 *  affect the interrupt I/O bits; the low 8 bits
			 *  on write turn on and off LEDs.  (There are no
			 *  LEDs in the emulator, so those bits are just
			 *  ignored.)
			 */
			int old_assert = (d->csr[0] & d->csr[2])? 1 : 0;
			int new_assert;
			/* fatal("[ kn02: write to CSR: 0x%08x ]\n", idata); */

			d->csr[1] = (idata >> 8) & 255;
			d->csr[2] = (idata >> 16) & 255;

			/*  Recalculate interrupt assertions:  */
			new_assert = (d->csr[0] & d->csr[2])? 1 : 0;
			if (new_assert != old_assert) {
				if (new_assert) {
					INTERRUPT_ASSERT(d->irq);
					d->int_asserted = 1;
				} else {
					INTERRUPT_DEASSERT(d->irq);
					d->int_asserted = 0;
				}
			}
		}
		break;
	default:
		if (writeflag==MEM_READ) {
			debug("[ kn02: read from 0x%08lx ]\n",
			    (long)relative_addr);
		} else {
			debug("[ kn02: write to  0x%08lx: 0x%08x ]\n",
			    (long)relative_addr, (int)idata);
		}
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVINIT(kn02)
{
	struct kn02_data *d;
	uint32_t csr;
	int i;

	CHECK_ALLOCATION(d = (struct kn02_data *) malloc(sizeof(struct kn02_data)));
	memset(d, 0, sizeof(struct kn02_data));

	/*  Connect the KN02 to a specific MIPS CPU interrupt line:  */
	INTERRUPT_CONNECT(devinit->interrupt_path, d->irq);

	/*  Register the 8 possible TurboChannel interrupts:  */
	for (i=0; i<8; i++) {
		struct interrupt templ;
		char tmpstr[300];
		snprintf(tmpstr, sizeof(tmpstr), "%s.kn02.%i",
		    devinit->interrupt_path, i);
		memset(&templ, 0, sizeof(templ));
		templ.line = 1 << i;
		templ.name = tmpstr;
		templ.extra = d;
		templ.interrupt_assert = kn02_interrupt_assert;
		templ.interrupt_deassert = kn02_interrupt_deassert;
		interrupt_handler_register(&templ);
	}

	/*
	 *  Set initial value of the CSR. Note: If the KN02_CSR_NRMMOD bit
	 *  is not set, the 5000/200 PROM image loops forever.
	 */
	csr = KN02_CSR_NRMMOD;
	d->csr[0] = csr;
	d->csr[1] = csr >> 8;
	d->csr[2] = csr >> 16;
	d->csr[3] = csr >> 24;

	memory_device_register(devinit->machine->memory, devinit->name,
	    devinit->addr, DEV_KN02_LENGTH, dev_kn02_access, d,
	    DM_DYNTRANS_OK, &d->csr[0]);

	return 1;
}

