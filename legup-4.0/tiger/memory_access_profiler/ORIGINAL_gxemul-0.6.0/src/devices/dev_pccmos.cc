/*
 *  Copyright (C) 2005-2009  Anders Gavare.  All rights reserved.
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
 *  COMMENT: PC CMOS/RTC device (ISA ports 0x70 and 0x71)
 *
 *  The main point of this device is to be a "PC style wrapper" for accessing
 *  the MC146818 (the RTC). In most other respects, this device is bogus, and
 *  just acts as a 256-byte RAM device.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "device.h"
#include "devices.h"
#include "emul.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"


#define	DEV_PCCMOS_LENGTH		2
#define	PCCMOS_MC146818_FAKE_ADDR	0x1d00000000ULL

struct pccmos_data {
	unsigned char	select;
	unsigned char	ram[256];
};


DEVICE_ACCESS(pccmos)
{
	struct pccmos_data *d = (struct pccmos_data *) extra;
	uint64_t idata = 0, odata = 0;
	unsigned char b = 0;
	int r = 1;

	if (writeflag == MEM_WRITE)
		b = idata = memory_readmax64(cpu, data, len);

	/*
	 *  Accesses to CMOS register 0 .. 0xd are rerouted to the
	 *  RTC; all other access are treated as CMOS RAM read/writes.
	 */

	if ((relative_addr & 1) == 0) {
		if (writeflag == MEM_WRITE) {
			d->select = idata;
			if (idata <= 0x0d) {
				r = cpu->memory_rw(cpu, cpu->mem,
				    PCCMOS_MC146818_FAKE_ADDR, &b, 1,
				    MEM_WRITE, PHYSICAL);
			}
		} else
			odata = d->select;
	} else {
		if (d->select <= 0x0d) {
			if (writeflag == MEM_WRITE) {
				r = cpu->memory_rw(cpu, cpu->mem,
				    PCCMOS_MC146818_FAKE_ADDR + 1, &b, 1,
				    MEM_WRITE, PHYSICAL);
			} else {
				r = cpu->memory_rw(cpu, cpu->mem,
				    PCCMOS_MC146818_FAKE_ADDR + 1, &b, 1,
				    MEM_READ, PHYSICAL);
				odata = b;
			}
		} else {
			if (writeflag == MEM_WRITE)
				d->ram[d->select] = idata;
			else
				odata = d->ram[d->select];
		}
	}

	if (r == 0)
		fatal("[ pccmos: memory_rw() error! ]\n");

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVINIT(pccmos)
{
	int type = MC146818_PC_CMOS, len = DEV_PCCMOS_LENGTH;
	struct pccmos_data *d;

	CHECK_ALLOCATION(d = (struct pccmos_data *) malloc(sizeof(struct pccmos_data)));
	memset(d, 0, sizeof(struct pccmos_data));

	switch (devinit->machine->machine_type) {
	case MACHINE_CATS:
	case MACHINE_NETWINDER:
		type = MC146818_CATS;
		d->ram[0x48] = 20;		/*  century  */
		len = DEV_PCCMOS_LENGTH * 2;
		break;
	case MACHINE_ALGOR:
		type = MC146818_ALGOR;
		break;
	case MACHINE_ARC:
		fatal("\nARC pccmos: TODO\n\n");
		type = MC146818_ALGOR;
		break;
	case MACHINE_EVBMIPS:
		/*  Malta etc.  */
		type = MC146818_ALGOR;
		break;
	case MACHINE_QEMU_MIPS:
	case MACHINE_COBALT:
	case MACHINE_PREP:
	case MACHINE_MVMEPPC:
	case MACHINE_IYONIX:	// TODO: not sure about exact type.
		break;
	default:fatal("devinit_pccmos(): unimplemented machine type"
		    " %i\n", devinit->machine->machine_type);
		exit(1);
	}

	memory_device_register(devinit->machine->memory, devinit->name,
	    devinit->addr, len, dev_pccmos_access, (void *)d,
	    DM_DEFAULT, NULL);

	dev_mc146818_init(devinit->machine, devinit->machine->memory,
	    PCCMOS_MC146818_FAKE_ADDR, devinit->interrupt_path, type, 1);

	return 1;
}

