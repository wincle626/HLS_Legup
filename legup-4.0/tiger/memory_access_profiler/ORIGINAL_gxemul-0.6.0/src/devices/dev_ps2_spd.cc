/*
 *  Copyright (C) 2004-2009  Anders Gavare.  All rights reserved.
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
 *  COMMENT: PlayStation 2 SPD harddisk controller
 *
 *  TODO
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "device.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"


#define	DEV_PS2_SPD_LENGTH		0x800


struct ps2_spd_data {
	uint64_t		wdcaddr;
};


DEVICE_ACCESS(ps2_spd)
{
	struct ps2_spd_data *d = (struct ps2_spd_data *) extra;
	uint64_t idata = 0, odata = 0;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	switch (relative_addr) {
	case 0x40:
	case 0x42:
	case 0x44:
	case 0x46:
	case 0x48:
	case 0x4a:
	case 0x4c:
	case 0x4e:
		debug("[ ps2_spd: wdc access ]\n");
		cpu->memory_rw(cpu, mem, (relative_addr - 0x40) / 2 +
		    d->wdcaddr, data, len, writeflag, PHYSICAL);
		return 1;
	case 0x5c:	/*  aux control  */
		debug("[ ps2_spd: wdc access (2) ]\n");
		cpu->memory_rw(cpu, mem, d->wdcaddr + 0x206, data, len,
		    writeflag, PHYSICAL);
		return 1;
	default:
		if (writeflag==MEM_READ) {
			debug("[ ps2_spd: read from addr 0x%x: 0x%llx ]\n",
			    (int)relative_addr, (long long)odata);
		} else {
			debug("[ ps2_spd: write to addr 0x%x: 0x%llx ]\n",
			    (int)relative_addr, (long long)idata);
		}
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVINIT(ps2_spd)
{
	struct ps2_spd_data *d;
	char tmpstr[200];

	CHECK_ALLOCATION(d = (struct ps2_spd_data *) malloc(sizeof(struct ps2_spd_data)));
	memset(d, 0, sizeof(struct ps2_spd_data));

	d->wdcaddr = devinit->addr + DEV_PS2_SPD_LENGTH;

	memory_device_register(devinit->machine->memory, "ps2_spd",
	    devinit->addr, DEV_PS2_SPD_LENGTH,
	    dev_ps2_spd_access, d, DM_DEFAULT, NULL);

	/*  Register a generic wdc device at a bogus address:  */
	/*  IRQ = PS2 SBUS irq 0  */
	snprintf(tmpstr, sizeof(tmpstr), "wdc addr=0x%llx irq=%s.ps2_sbus.0",
	    (long long)d->wdcaddr, devinit->interrupt_path);
	device_add(devinit->machine, tmpstr);

	return 1;
}

