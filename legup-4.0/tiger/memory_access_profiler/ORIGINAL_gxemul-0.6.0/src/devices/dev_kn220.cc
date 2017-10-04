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
 *  COMMENT: DEC KN220 (DECsystem 5500) devices
 *
 *	o)  I/O board
 *	o)  SGEC (ethernet)  (Called "ne" in Ultrix.)
 *
 *  TODO:  Study docs.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "devices.h"
#include "memory.h"
#include "misc.h"

#define IOBOARD_DEBUG

struct dec5500_ioboard_data {
	int	dummy;
};

#define SGEC_DEBUG

struct sgec_data {
	int	irq_nr;
};


/*
 *  dev_dec5500_ioboard_access():
 */
DEVICE_ACCESS(dec5500_ioboard)
{
	/*  struct dec5500_ioboard_data *d =
	    (struct dec5500_ioboard_data *) extra;  */
	uint64_t idata = 0, odata = 0;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

#ifdef IOBOARD_DEBUG
	if (writeflag == MEM_WRITE)
		debug("[ dec5500_ioboard: write to address 0x%llx, "
		    "data=0x%016llx ]\n", (long long)relative_addr,
		    (long long)idata);
	else
		debug("[ dec5500_ioboard: read from address 0x%llx ]\n",
		    (long long)relative_addr);
#endif

	switch (relative_addr) {
	case 0:
		if (writeflag == MEM_READ)
			/*
			 *  TODO:  One of these bits indicate I/O board
			 *  present.
			 */
			odata = 0xffffffffULL;
		break;

	default:
		if (writeflag == MEM_WRITE)
			debug("[ dec5500_ioboard: unimplemented write to "
			    "address 0x%llx, data=0x%016llx ]\n",
			    (long long)relative_addr, (long long)idata);
		else
			debug("[ dec5500_ioboard: unimplemented read from"
			    " address 0x%llx ]\n", (long long)relative_addr);
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


/*
 *  dev_sgec_access():
 */
DEVICE_ACCESS(sgec)
{
	/*  struct sgec_data *d = (struct sgec_data *) extra;  */
	uint64_t idata = 0, odata = 0;

	idata = memory_readmax64(cpu, data, len);

#ifdef SGEC_DEBUG
	if (writeflag == MEM_WRITE)
		debug("[ sgec: write to address 0x%llx, data=0x%016llx ]\n",
		    (long long)relative_addr, (long long)idata);
	else
		debug("[ sgec: read from address 0x%llx ]\n",
		    (long long)relative_addr);
#endif

	switch (relative_addr) {
	case 0x14:
		if (writeflag == MEM_READ)
			odata = 0x80000000;
		break;

	default:
		if (writeflag == MEM_WRITE)
			debug("[ sgec: unimplemented write to address 0x%llx,"
			    " data=0x%016llx ]\n", (long long)relative_addr,
			    (long long)idata);
		else
			debug("[ sgec: unimplemented read from address "
			    "0x%llx ]\n", (long long)relative_addr);
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


/*
 *  dev_sgec_init():
 */
void dev_sgec_init(struct memory *mem, uint64_t baseaddr, int irq_nr)
{
	struct sgec_data *d;

	CHECK_ALLOCATION(d = (struct sgec_data *) malloc(sizeof(struct sgec_data)));
	memset(d, 0, sizeof(struct sgec_data));

	d->irq_nr = irq_nr;

	memory_device_register(mem, "sgec", baseaddr, DEV_SGEC_LENGTH,
	    dev_sgec_access, (void *)d, DM_DEFAULT, NULL);
}


/*
 *  dev_dec5500_ioboard_init():
 */
struct dec5500_ioboard_data *dev_dec5500_ioboard_init(struct cpu *cpu,
	struct memory *mem, uint64_t baseaddr)
{
	struct dec5500_ioboard_data *d;

	CHECK_ALLOCATION(d = (struct dec5500_ioboard_data *) malloc(sizeof(struct dec5500_ioboard_data)));
	memset(d, 0, sizeof(struct dec5500_ioboard_data));

	memory_device_register(mem, "dec5500_ioboard", baseaddr,
	    DEV_DEC5500_IOBOARD_LENGTH, dev_dec5500_ioboard_access,
	    (void *)d, DM_DEFAULT, NULL);

	return d;
}

