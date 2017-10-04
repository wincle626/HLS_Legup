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
 *  COMMENT: IQ80321 LED device
 *
 *  Should work with NetBSD's iq80321_7seg.c.
 *
 *  TODO: Graphical output of LED lines?
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "device.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"


struct iq80321_7seg_data {
	uint8_t		msb, lsb;
};


DEVICE_ACCESS(iq80321_7seg)
{
	struct iq80321_7seg_data *d = (struct iq80321_7seg_data *) extra;
	uint64_t idata = 0, odata = 0;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	switch (relative_addr) {

	case 0:	if (writeflag == MEM_READ)
			odata = d->msb;
		else {
			d->msb = idata;
			debug("[ iq80321_7seg: setting MSB to 0x%02x ]\n",
			    (int)idata);
		}
		break;

	case 0x10000:
		if (writeflag == MEM_READ)
			odata = d->lsb;
		else {
			d->lsb = idata;
			debug("[ iq80321_7seg: setting LSB to 0x%02x ]\n",
			    (int)idata);
		}
		break;

	default:if (writeflag == MEM_READ) {
			fatal("[ iq80321_7seg: read from 0x%x ]\n",
			    (int)relative_addr);
		} else {
			fatal("[ iq80321_7seg: write to 0x%x: 0x%llx ]\n",
			    (int)relative_addr, (long long)idata);
		}
		exit(1);
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVINIT(iq80321_7seg)
{
	struct iq80321_7seg_data *d;

	CHECK_ALLOCATION(d = (struct iq80321_7seg_data *) malloc(sizeof(struct iq80321_7seg_data)));
	memset(d, 0, sizeof(struct iq80321_7seg_data));

	/*  0xfe840000 and 0xfe850000  */
	memory_device_register(devinit->machine->memory, devinit->name,
	    devinit->addr, 0x10004, dev_iq80321_7seg_access, d,
	    DM_DEFAULT, NULL);

	return 1;
}


