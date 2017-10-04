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
 *  COMMENT: National Semiconductor SONIC ("sn") DP83932 ethernet controller
 *
 *  TODO
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
#include "net.h"

#include "thirdparty/dp83932reg.h"


#define	DEV_SN_LENGTH		0x1000

struct sn_data {
	struct interrupt irq;
	unsigned char	macaddr[6];
	uint32_t	reg[SONIC_NREGS];
};


DEVICE_ACCESS(sn)
{
	struct sn_data *d = (struct sn_data *) extra;
	uint64_t idata = 0, odata = 0;
	int regnr;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	regnr = relative_addr / sizeof(uint32_t);

	if (regnr < SONIC_NREGS) {
		if (writeflag == MEM_WRITE)
			d->reg[regnr] = idata;
		else
			odata = d->reg[regnr];
	}

	switch (regnr) {

	default:
		if (writeflag == MEM_WRITE) {
			fatal("[ sn: unimplemented write to address 0x%x"
			    " (regnr %i), data=0x%02x ]\n",
			    (int)relative_addr, regnr, (int)idata);
		} else {
			fatal("[ sn: unimplemented read from address 0x%x "
			    "(regnr %i) ]\n", (int)relative_addr, regnr);
		}
		/*  exit(1);  */
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVINIT(sn)
{
	char *name2;
	size_t nlen = 55;
	struct sn_data *d;

	CHECK_ALLOCATION(d = (struct sn_data *) malloc(sizeof(struct sn_data)));
	memset(d, 0, sizeof(struct sn_data));

	INTERRUPT_CONNECT(devinit->interrupt_path, d->irq);

	net_generate_unique_mac(devinit->machine, d->macaddr);

	CHECK_ALLOCATION(name2 = (char *) malloc(nlen));
	snprintf(name2, nlen, "%s [%02x:%02x:%02x:%02x:%02x:%02x]",
	    devinit->name, d->macaddr[0], d->macaddr[1], d->macaddr[2],
	    d->macaddr[3], d->macaddr[4], d->macaddr[5]);

	memory_device_register(devinit->machine->memory, name2,
	    devinit->addr, DEV_SN_LENGTH,
	    dev_sn_access, (void *)d, DM_DEFAULT, NULL);

	net_add_nic(devinit->machine->emul->net, d, d->macaddr);

	return 1;
}

