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
 *  COMMENT: SGI IP19 (and IP25) stuff
 *
 *  NOTE/TODO: The stuff in here is mostly guesswork.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "device.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"


#define	DEV_SGI_IP19_LENGTH	0x100000

struct sgi_ip19_data {
	uint64_t	cycle_counter;
};


DEVICE_ACCESS(sgi_ip19)
{
	struct sgi_ip19_data *d = (struct sgi_ip19_data *) extra;
	uint64_t idata = 0, odata = 0;
	int regnr;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	regnr = relative_addr / sizeof(uint32_t);

	switch (relative_addr) {

	case 0x08:	/*  cpu id  */
		if (writeflag == MEM_WRITE) {
			debug("[ sgi_ip19: unimplemented write to address "
			    "0x%x (cpu id), data=0x%08x ]\n", (int)
			    relative_addr, (int)idata);
		} else {
			odata = cpu->cpu_id;	/*  ?  TODO  */
		}
		break;

	case 0x200:	/*  cpu available mask?  */
		if (writeflag == MEM_WRITE) {
			debug("[ sgi_ip19: unimplemented write to address "
			    "0x%x (cpu available mask), data=0x%08x ]\n",
			    (int)relative_addr, (int)idata);
		} else {
			/*  Max 16 cpus?  */
			odata = ((1 << cpu->machine->ncpus) - 1) << 16;
		}
		break;

	case 0x20000:	/*  cycle counter or clock  */
		if (writeflag == MEM_WRITE) {
			debug("[ sgi_ip19: unimplemented write to address "
			    "0x%x (cycle counter), data=0x%08x ]\n",
			    (int)relative_addr, (int)idata);
		} else {
			d->cycle_counter += 100;
			odata = d->cycle_counter;
		}

		break;

	default:
		if (writeflag == MEM_WRITE) {
			debug("[ sgi_ip19: unimplemented write to address "
			    "0x%x, data=0x%08x ]\n", (int)relative_addr,
			    (int)idata);
		} else {
			debug("[ sgi_ip19: unimplemented read from address "
			    "0x%x: 0x%08x ]\n", (int)relative_addr, (int)odata);
		}
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVINIT(sgi_ip19)
{
	struct sgi_ip19_data *d;

	CHECK_ALLOCATION(d = (struct sgi_ip19_data *) malloc(sizeof(struct sgi_ip19_data)));
	memset(d, 0, sizeof(struct sgi_ip19_data));

	memory_device_register(devinit->machine->memory, devinit->name,
	    devinit->addr, DEV_SGI_IP19_LENGTH,
	    dev_sgi_ip19_access, (void *)d, DM_DEFAULT, NULL);

	return 1;
}

