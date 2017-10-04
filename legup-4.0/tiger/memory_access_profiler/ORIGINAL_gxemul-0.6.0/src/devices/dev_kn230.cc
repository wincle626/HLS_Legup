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
 *  COMMENT: DEC KN230 (MIPSMATE 5100) stuff
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "device.h"
#include "interrupt.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"

#include "thirdparty/dec_5100.h"


#define	DEV_KN230_LENGTH		0x1c00000

struct kn230_data {
	struct interrupt	mips_irq_2;
	struct interrupt	mips_irq_3;

	uint32_t		csr;
};


/*  
 *  kn230_interrupt_assert():
 *  kn230_interrupt_deassert():
 *
 *  Called whenever a KN230 interrupt is asserted/deasserted.
 */
void kn230_interrupt_assert(struct interrupt *interrupt)
{
	struct kn230_data *d = (struct kn230_data *) interrupt->extra;
	int assert2 = 0, assert3 = 0;

	d->csr |= interrupt->line;
	if (d->csr & (KN230_CSR_INTR_SII | KN230_CSR_INTR_LANCE))
		assert3 = 1;
	if (d->csr & (KN230_CSR_INTR_DZ0 |
	    KN230_CSR_INTR_OPT0 | KN230_CSR_INTR_OPT1))
		assert2 = 1;

	if (assert2)
                INTERRUPT_ASSERT(d->mips_irq_2);
	if (assert3)
                INTERRUPT_ASSERT(d->mips_irq_2);
}
void kn230_interrupt_deassert(struct interrupt *interrupt)
{
	struct kn230_data *d = (struct kn230_data *) interrupt->extra;
	int assert2 = 0, assert3 = 0;

	d->csr &= ~interrupt->line;
	if (d->csr & (KN230_CSR_INTR_SII | KN230_CSR_INTR_LANCE))
		assert3 = 1;
	if (d->csr & (KN230_CSR_INTR_DZ0 |
	    KN230_CSR_INTR_OPT0 | KN230_CSR_INTR_OPT1))
		assert2 = 1;

	if (!assert2)
                INTERRUPT_DEASSERT(d->mips_irq_2);
	if (!assert3)
                INTERRUPT_DEASSERT(d->mips_irq_2);
}


DEVICE_ACCESS(kn230)
{
	struct kn230_data *d = (struct kn230_data *) extra;
	uint64_t idata = 0, odata = 0;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	switch (relative_addr) {
	case 0:
		if (writeflag==MEM_READ) {
			odata = d->csr;
			/* debug("[ kn230: read from CSR: 0x%08x ]\n",
			    (int)odata); */
		} else {
			/* debug("[ kn230: write to CSR: 0x%08x ]\n",
			    (int)idata); */
		}
		break;
	default:
		if (writeflag==MEM_READ) {
			debug("[ kn230: read from 0x%08lx ]\n",
			    (long)relative_addr);
		} else {
			debug("[ kn230: write to  0x%08lx: 0x%08x ]\n",
			    (long)relative_addr, (int)idata);
		}
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);
  
	return 1;
}


DEVINIT(kn230)
{
	struct kn230_data *d;
	char tmpstr[300];
	int i;

	CHECK_ALLOCATION(d = (struct kn230_data *) malloc(sizeof(struct kn230_data)));
	memset(d, 0, sizeof(struct kn230_data));

	/*
	 *  devinit->interrupt_path points to the MIPS cpu itself.
	 *  The KN230 interrupt controller interrupts at MIPS interrupts
	 *  2 and 3, depending on which KN230 is asserted.
	 */
	snprintf(tmpstr, sizeof(tmpstr), "%s.2", devinit->interrupt_path);
	INTERRUPT_CONNECT(tmpstr, d->mips_irq_2);
	snprintf(tmpstr, sizeof(tmpstr), "%s.3", devinit->interrupt_path);
	INTERRUPT_CONNECT(tmpstr, d->mips_irq_3);

	/*  Register KN230 interrupts 8..15:  */
	for (i=8; i<=15; i++) {
		struct interrupt templ;
		char tmpstr[300];
		snprintf(tmpstr, sizeof(tmpstr), "%s.kn230.0x%x",
		    devinit->interrupt_path, 1 << i);
		memset(&templ, 0, sizeof(templ));
		templ.line = 1 << i;
		templ.name = tmpstr;
		templ.extra = d;
                templ.interrupt_assert = kn230_interrupt_assert;
                templ.interrupt_deassert = kn230_interrupt_deassert;
		interrupt_handler_register(&templ);
	}

	memory_device_register(devinit->machine->memory, devinit->name,
	    devinit->addr, DEV_KN230_LENGTH, dev_kn230_access, d,
	    DM_DEFAULT, NULL);

	devinit->return_ptr = d;

	return 1;
}

