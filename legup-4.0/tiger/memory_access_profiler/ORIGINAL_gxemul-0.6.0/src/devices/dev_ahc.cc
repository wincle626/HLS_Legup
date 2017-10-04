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
 *  COMMENT: Adaptec AHC SCSI controller
 *
 *  NetBSD should say something like this, on SGI-IP32:
 *	ahc0 at pci0 dev 1 function 0
 *	ahc0: interrupting at crime irq 0
 *	ahc0: aic7880 Wide Channel A, SCSI Id=7, 16/255 SCBs
 *	ahc0: Host Adapter Bios disabled.  Using default SCSI device parameters
 *
 *  TODO:  This more or less just a dummy device, so far.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "device.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"

#include "thirdparty/aic7xxx_reg.h"


/* #define	AHC_DEBUG
   #define debug fatal */


#define	DEV_AHC_LENGTH		0x100

struct ahc_data {
	unsigned char	reg[DEV_AHC_LENGTH];
};


DEVICE_ACCESS(ahc)
{
	struct ahc_data *d = (struct ahc_data *) extra;
	uint64_t idata, odata = 0;
	int ok = 0;
	const char *name = NULL;

	idata = memory_readmax64(cpu, data, len);

	/*  YUCK! SGI uses reversed order inside 32-bit words:  */
	if (cpu->byte_order == EMUL_BIG_ENDIAN)
		relative_addr = (relative_addr & ~0x3)
		    | (3 - (relative_addr & 3));

	relative_addr %= DEV_AHC_LENGTH;

	if (len != 1)
		fatal("[ ahc: ERROR! Unimplemented len %i ]\n", len);

	if (writeflag == MEM_READ)
		odata = d->reg[relative_addr];

	switch (relative_addr) {

	case SCSIID:
		if (writeflag == MEM_READ) {
			ok = 1; name = "SCSIID";
			odata = 0;
		} else {
			fatal("[ ahc: write to SCSIOFFSET, data = 0x"
			    "%02x: TODO ]\n", (int)idata);
		}
		break;

	case KERNEL_QINPOS:
		if (writeflag == MEM_WRITE) {

			/*  TODO  */

			d->reg[INTSTAT] |= SEQINT;
		}
		break;

	case SEECTL:
		ok = 1; name = "SEECTL";
		if (writeflag == MEM_WRITE)
			d->reg[relative_addr] = idata;
		odata |= SEERDY;
		break;

	case SCSICONF:
		ok = 1; name = "SCSICONF";
		if (writeflag == MEM_READ) {
			odata = 0;
		} else {
			fatal("[ ahc: write to SCSICONF, data = 0x%02x:"
			    " TODO ]\n", (int)idata);
		}
		break;

	case SEQRAM:
	case SEQADDR0:
	case SEQADDR1:
		/*  TODO: This is just a dummy.  */
		break;

	case HCNTRL:
		ok = 1; name = "HCNTRL";
		if (writeflag == MEM_WRITE)
			d->reg[relative_addr] = idata;
		break;

	case INTSTAT:
		ok = 1; name = "INTSTAT";
		if (writeflag == MEM_WRITE)
			fatal("[ ahc: write to INTSTAT? data = 0x%02x ]\n",
			    (int)idata);
		break;

	case CLRINT:
		if (writeflag == MEM_READ) {
			ok = 1; name = "ERROR";
			/*  TODO  */
		} else {
			ok = 1; name = "CLRINT";
			if (idata & ~0xf)
				fatal("[ ahc: write to CLRINT: 0x%02x "
				    "(TODO) ]\n", (int)idata);
			/*  Clear the lowest 4 bits of intstat:  */
			d->reg[INTSTAT] &= ~(idata & 0xf);
		}
		break;

	default:
		if (writeflag == MEM_WRITE)
			fatal("[ ahc: UNIMPLEMENTED write to address 0x%x, "
			    "data=0x%02x ]\n", (int)relative_addr, (int)idata);
		else
			fatal("[ ahc: UNIMPLEMENTED read from address 0x%x ]\n",
			    (int)relative_addr);
	}

#ifdef AHC_DEBUG
	if (ok) {
		if (name == NULL) {
			if (writeflag == MEM_WRITE)
				debug("[ ahc: write to address 0x%x: 0x"
				    "%02x ]\n", (int)relative_addr, (int)idata);
			else
				debug("[ ahc: read from address 0x%x: 0x"
				    "%02x ]\n", (int)relative_addr, (int)odata);
		} else {
			if (writeflag == MEM_WRITE)
				debug("[ ahc: write to %s: 0x%02x ]\n",
				    name, (int)idata);
			else
				debug("[ ahc: read from %s: 0x%02x ]\n",
				    name, (int)odata);
		}
	}
#endif

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVINIT(ahc)
{
	struct ahc_data *d;

	CHECK_ALLOCATION(d = (struct ahc_data *) malloc(sizeof(struct ahc_data)));
	memset(d, 0, sizeof(struct ahc_data));

	memory_device_register(devinit->machine->memory, devinit->name,
	    devinit->addr, DEV_AHC_LENGTH, dev_ahc_access, d,
	    DM_DEFAULT, NULL);

	return 1;
}

