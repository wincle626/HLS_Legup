/*
 *  Copyright (C) 2006-2009  Anders Gavare.  All rights reserved.
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
 *  COMMENT: Dreamcast-specific ASIC
 *
 *  A simple device which forwards various Dreamcast device events as
 *  interrupts 13, 11, or 9, to the CPU.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "cpu.h"
#include "device.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"

#include "thirdparty/dreamcast_sysasicvar.h"
#include "thirdparty/sh4_exception.h"


#define debug fatal

#define	DREAMCAST_ASIC_TICK_SHIFT	15

struct dreamcast_asic_data {
	uint32_t	pending_irq[3];
	uint32_t	mask_13[3];
	uint32_t	mask_11[3];
	uint32_t	mask_9[3];

	int		asserted_13;
	int		asserted_11;
	int		asserted_9;

	struct interrupt irq_13;
	struct interrupt irq_11;
	struct interrupt irq_9;
};


DEVICE_TICK(dreamcast_asic)
{
	struct dreamcast_asic_data *d = (struct dreamcast_asic_data *) extra;
	int i, old_asserted_13 = d->asserted_13, old_asserted_11 =
	    d->asserted_11, old_asserted_9 = d->asserted_9;

	d->asserted_13 = d->asserted_11 = d->asserted_9 = 0;

	for (i=0; i<3; i++) {
		if (d->pending_irq[i] & d->mask_13[i])
			d->asserted_13 = 1;

		if (d->pending_irq[i] & d->mask_11[i])
			d->asserted_11 = 1;

		if (d->pending_irq[i] & d->mask_9[i])
			d->asserted_9 = 1;
	}

	if (d->asserted_13 != old_asserted_13) {
		if (d->asserted_13)
			INTERRUPT_ASSERT(d->irq_13);
		else
			INTERRUPT_DEASSERT(d->irq_13);
	}
	if (d->asserted_11 != old_asserted_11) {
		if (d->asserted_11)
			INTERRUPT_ASSERT(d->irq_11);
		else
			INTERRUPT_DEASSERT(d->irq_11);
	}
	if (d->asserted_9 != old_asserted_9) {
		if (d->asserted_9)
			INTERRUPT_ASSERT(d->irq_9);
		else
			INTERRUPT_DEASSERT(d->irq_9);
	}
}


DEVICE_ACCESS(dreamcast_asic)
{
	struct dreamcast_asic_data *d = (struct dreamcast_asic_data *) extra;
	uint64_t idata = 0, odata = 0;
	int r;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	r = (relative_addr / 4) & 3;
	if (r == 3) {
		fatal("[ dreamcast_asic: Bad address ]\n");
		r = 0;
	}

	switch (relative_addr) {

	case 0:
	case 4:
	case 8:	if (writeflag == MEM_READ) {
			odata = d->pending_irq[r];
		} else {
			/*  Should only be used interally by GXemul:  */
			if (idata & 0x100000000ULL) {
				/*  Set specific bits:  */
				d->pending_irq[r] |= idata;
			} else {
				/*  Clear interrupt assertions:  */
				d->pending_irq[r] &= ~idata;
			}
			dev_dreamcast_asic_tick(cpu, d);
		}
		break;

	case 0x10:
	case 0x14:
	case 0x18:
		if (writeflag == MEM_WRITE) {
			d->mask_13[r] = idata;
			dev_dreamcast_asic_tick(cpu, d);
		} else {
			odata = d->mask_13[r];
		}
		break;

	case 0x20:
	case 0x24:
	case 0x28:
		if (writeflag == MEM_WRITE) {
			d->mask_11[r] = idata;
			dev_dreamcast_asic_tick(cpu, d);
		} else {
			odata = d->mask_11[r];
		}
		break;

	case 0x30:
	case 0x34:
	case 0x38:
		if (writeflag == MEM_WRITE) {
			d->mask_9[r] = idata;
			dev_dreamcast_asic_tick(cpu, d);
		} else {
			odata = d->mask_9[r];
		}
		break;

	default:if (writeflag == MEM_READ) {
			fatal("[ dreamcast_asic: read from addr 0x%x ]\n",
			    (int)relative_addr);
		} else {
			fatal("[ dreamcast_asic: write to addr 0x%x: 0x%x ]\n",
			    (int)relative_addr, (int)idata);
		}
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVINIT(dreamcast_asic)
{
	char tmpstr[300];
	struct machine *machine = devinit->machine;
	struct dreamcast_asic_data *d;

	CHECK_ALLOCATION(d = (struct dreamcast_asic_data *) malloc(sizeof(struct dreamcast_asic_data)));
	memset(d, 0, sizeof(struct dreamcast_asic_data));

	/*  Connect to SH4 interrupt levels 13, 11, and 9:  */
	snprintf(tmpstr, sizeof(tmpstr), "%s.irq[0x%x]",
	    devinit->interrupt_path, SH_INTEVT_IRL13);
	INTERRUPT_CONNECT(tmpstr, d->irq_13);
	snprintf(tmpstr, sizeof(tmpstr), "%s.irq[0x%x]",
	    devinit->interrupt_path, SH_INTEVT_IRL11);
	INTERRUPT_CONNECT(tmpstr, d->irq_11);
	snprintf(tmpstr, sizeof(tmpstr), "%s.irq[0x%x]",
	    devinit->interrupt_path, SH_INTEVT_IRL9);
	INTERRUPT_CONNECT(tmpstr, d->irq_9);

	memory_device_register(machine->memory, devinit->name, SYSASIC_BASE,
	    SYSASIC_SIZE, dev_dreamcast_asic_access, d, DM_DEFAULT, NULL);

	machine_add_tickfunction(devinit->machine, dev_dreamcast_asic_tick, d,
	    DREAMCAST_ASIC_TICK_SHIFT);

	return 1;
}

