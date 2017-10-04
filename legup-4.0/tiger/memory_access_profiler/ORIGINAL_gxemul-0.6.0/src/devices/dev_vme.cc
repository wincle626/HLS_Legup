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
 *  COMMENT: VME bus
 *
 *  TODO: Probably almost everything.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "device.h"
#include "emul.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"

#include "thirdparty/mvme88k_vme.h"


/*  #define debug fatal  */


#define	VME_LEN			0x1000

struct vme_data {
	uint32_t	reg[VME_LEN / sizeof(uint32_t)];
};


DEVICE_ACCESS(vme)
{
	struct vme_data *d = (struct vme_data *) extra;
	uint64_t idata = 0, odata = 0;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	if (writeflag == MEM_READ)
		odata = d->reg[relative_addr / sizeof(uint32_t)];

	switch (relative_addr) {

	case 0:	/*  Used by OpenBSD/mvme88k when probing...  */
		break;

	case VME2_GCSRCTL:
		debug("[ vme: unimplemented GCSRCTL ]\n");
		break;

	case VME2_TCR:
		if (writeflag == MEM_READ)
			debug("[ vme: unimplemented READ from TCR ]\n");
		else
			debug("[ vme: unimplemented WRITE to TCR: "
			    "0x%x ]\n", (int)idata);
		break;

	case VME2_T1CMP:
		if (writeflag == MEM_WRITE)
			d->reg[relative_addr / sizeof(uint32_t)] = idata;
		break;

	case VME2_T1COUNT:
		if (writeflag == MEM_WRITE)
			d->reg[relative_addr / sizeof(uint32_t)] = idata;
		else {
			/*  NOTE! This is a quick hack. TODO: Fix!  */
			d->reg[relative_addr / sizeof(uint32_t)] += 100;
		}
		break;

	case VME2_TCTL:
		if (writeflag == MEM_WRITE)
			d->reg[relative_addr / sizeof(uint32_t)] = idata;

		/*  TODO  */
		/*  debug("[ vme: unimplemented TCTL ]\n");  */
		break;

	case VME2_IRQEN:
		if (writeflag == MEM_READ)
			debug("[ vme: unimplemented READ from IRQEN ]\n");
		else
			debug("[ vme: unimplemented WRITE to IRQEN: "
			    "0x%x ]\n", (int)idata);
		break;

	case VME2_IRQL3:
		if (writeflag == MEM_READ)
			debug("[ vme: unimplemented READ from IRQL3 ]\n");
		else
			debug("[ vme: unimplemented WRITE to IRQL3: "
			    "0x%x ]\n", (int)idata);
		break;

	case VME2_IRQL4:
		if (writeflag == MEM_READ)
			debug("[ vme: unimplemented READ from IRQL4 ]\n");
		else
			debug("[ vme: unimplemented WRITE to IRQL4: "
			    "0x%x ]\n", (int)idata);
		break;

	case VME2_VBR:
		/*  Vector Base Register.  */
		if (writeflag == MEM_WRITE)
			d->reg[relative_addr / sizeof(uint32_t)] = idata;
		break;

	default:if (writeflag == MEM_READ)
			debug("[ vme: unimplemented READ from offset 0x%x ]"
			    "\n", (int)relative_addr);
		else
			debug("[ vme: unimplemented WRITE to offset 0x%x: "
			    "0x%x ]\n", (int)relative_addr, (int)idata);
		exit(1);
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVINIT(vme)
{
	struct vme_data *d;

	CHECK_ALLOCATION(d = (struct vme_data *) malloc(sizeof(struct vme_data)));
	memset(d, 0, sizeof(struct vme_data));

	/*  According to OpenBSD/mvme88k:  */
	d->reg[VME2_VBR / sizeof(uint32_t)] =
	    VME2_SET_VBR0(6) + VME2_SET_VBR1(7);

	memory_device_register(devinit->machine->memory, devinit->name,
	    devinit->addr, VME_LEN, dev_vme_access, (void *)d,
	    DM_DEFAULT, NULL);

	return 1;
}

