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
 *  COMMENT: SGI IP30 stuff
 *
 *  NOTE/TODO: This is just comprised of hardcoded guesses so far. (Ugly.)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "device.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"


#define	DEV_SGI_IP30_LENGTH		0x80000

struct sgi_ip30_data {
	/*  ip30:  */
	uint64_t                imask0;         /*  0x10000  */
	uint64_t                reg_0x10018;
	uint64_t                isr;            /*  0x10030  */
	uint64_t                reg_0x20000;
	uint64_t                reg_0x30000;

	/*  ip30_2:  */
	uint64_t                reg_0x0029c;

	/*  ip30_3:  */
	uint64_t                reg_0x00284;

	/*  ip30_4:  */
	uint64_t                reg_0x000b0;

	/*  ip30_5:  */
	uint64_t                reg_0x00000;
};


DEVICE_TICK(sgi_ip30)
{
	struct sgi_ip30_data *d = (struct sgi_ip30_data *) extra;

	d->reg_0x20000 += 1000;

	if (d->imask0 & ((int64_t)1<<50)) {
		/*  TODO: Only interrupt if reg 0x20000 (the counter)
			has passed the compare (0x30000).  */
fatal("IP30 legacy interrupt rewrite: TODO\n");
abort();
//		cpu_interrupt(cpu, 8+1 + 50);
	}
}


DEVICE_ACCESS(sgi_ip30)
{
	struct sgi_ip30_data *d = (struct sgi_ip30_data *) extra;
	uint64_t idata = 0, odata = 0;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	switch (relative_addr) {

	case 0x20:
		/*  Memory bank configuration:  */
		odata = 0x80010000ULL;
		break;

	case 0x10000:	/*  Interrupt mask register 0:  */
		if (writeflag == MEM_WRITE) {
			d->imask0 = idata;
		} else {
			odata = d->imask0;
		}
		break;

	case 0x10018:
		/*
		 *  If this is not implemented, the IP30 PROM complains during
		 *  bootup:
		 *
		 *           *FAILED*
		 *    Address: 0xffffffffaff10018, Expected:
		 *        0x0000000000000001, Received: 0x0000000000000000
		 */
		if (writeflag == MEM_WRITE) {
			d->reg_0x10018 = idata;
		} else {
			odata = d->reg_0x10018;
		}
		break;

	case 0x10020:	/*  Set ISR, according to Linux/IP30  */
		d->isr = idata;
		/*  Recalculate CPU interrupt assertions:  */
fatal("IP30 legacy interrupt rewrite: TODO\n");
abort();
//		cpu_interrupt(cpu, 8);
		break;

	case 0x10028:	/*  Clear ISR, according to Linux/IP30  */
		d->isr &= ~idata;
		/*  Recalculate CPU interrupt assertions:  */
fatal("IP30 legacy interrupt rewrite: TODO\n");
abort();
//		cpu_interrupt(cpu, 8);
		break;

	case 0x10030:	/*  Interrupt Status Register  */
		if (writeflag == MEM_WRITE) {
			/*  Clear-on-write  (TODO: is this correct?)  */
			d->isr &= ~idata;
			/*  Recalculate CPU interrupt assertions:  */
fatal("IP30 legacy interrupt rewrite: TODO\n");
abort();
//			cpu_interrupt(cpu, 8);
		} else {
			odata = d->isr;
		}
		break;

	case 0x20000:
		/*  A counter  */
		if (writeflag == MEM_WRITE) {
			d->reg_0x20000 = idata;
		} else {
			odata = d->reg_0x20000;
		}
		break;

	case 0x30000:
		if (writeflag == MEM_WRITE) {
			d->reg_0x30000 = idata;
		} else {
			odata = d->reg_0x30000;
		}
		break;

	default:
		if (writeflag == MEM_WRITE) {
			debug("[ sgi_ip30: unimplemented write to address "
			    "0x%x, data=0x%02x ]\n", (int)relative_addr,
			    (int)idata);
		} else {
			debug("[ sgi_ip30: unimplemented read from address"
			    " 0x%x ]\n", (int)relative_addr);
		}
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVICE_ACCESS(sgi_ip30_2)
{
	struct sgi_ip30_data *d = (struct sgi_ip30_data *) extra;
	uint64_t idata = 0, odata = 0;

	idata = memory_readmax64(cpu, data, len);

	switch (relative_addr) {

	/*  0x114 + 0x40 * (wid - 8): 0x80000000 for "alive",
	    according to Linux/IP30  */

	case 0x114 + 0x40 * (12 - 8):
		fatal("[ IP30: asdvasdvnb ]\n");
		odata = 0x80000000;
		break;

	case 0x0029c:
		/*
		 *  If this is not implemented, the IP30 PROM complains during
		 *  bootup:
		 *
		 *           *FAILED*
		 *    Address: 0xffffffffb000029c, Expected:
		 *	0x0000000000000001, Received: 0x0000000000000000
		 */
		if (writeflag == MEM_WRITE) {
			d->reg_0x0029c = idata;
		} else {
			odata = d->reg_0x0029c;
		}
		break;
	default:
		if (writeflag == MEM_WRITE) {
			debug("[ sgi_ip30_2: unimplemented write to "
			    "address 0x%x, data=0x%02x ]\n",
			    (int)relative_addr, (int)idata);
		} else {
			debug("[ sgi_ip30_2: unimplemented read from address "
			    "0x%x ]\n", (int)relative_addr);
		}
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVICE_ACCESS(sgi_ip30_3)
{
	struct sgi_ip30_data *d = (struct sgi_ip30_data *) extra;
	uint64_t idata = 0, odata = 0;

	idata = memory_readmax64(cpu, data, len);

	switch (relative_addr) {

	case 0xb4:
		if (writeflag == MEM_WRITE) {
			debug("[ sgi_ip30_3: unimplemented write to "
			    "address 0x%x, data=0x%02x ]\n",
			    (int)relative_addr, (int)idata);
		} else {
			odata = 2;	/*  should be 2, or Irix loops  */
		}
		break;

	case 0x00104:
		if (writeflag == MEM_WRITE) {
			debug("[ sgi_ip30_3: unimplemented write to address "
			    "0x%x, data=0x%02x ]\n", (int)relative_addr,
			    (int)idata);
		} else {
			odata = 64;	/*  should be 64, or the PROM
					    complains  */
		}
		break;

	case 0x00284:
		/*
		 *  If this is not implemented, the IP30 PROM complains during
		 *  bootup:
		 *
		 *           *FAILED*
		 *    Address: 0xffffffffbf000284, Expected:
		 *	   0x0000000000000001, Received: 0x0000000000000000
		 */
		if (writeflag == MEM_WRITE) {
			d->reg_0x00284 = idata;
		} else {
			odata = d->reg_0x00284;
		}
		break;

	default:
		if (writeflag == MEM_WRITE) {
			debug("[ sgi_ip30_3: unimplemented write to address "
			    "0x%x, data=0x%02x ]\n", (int)relative_addr,
			    (int)idata);
		} else {
			debug("[ sgi_ip30_3: unimplemented read from "
			    "address 0x%x ]\n", (int)relative_addr);
		}
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVICE_ACCESS(sgi_ip30_4)
{
	struct sgi_ip30_data *d = (struct sgi_ip30_data *) extra;
	uint64_t idata = 0, odata = 0;

	idata = memory_readmax64(cpu, data, len);

	switch (relative_addr) {

	case 0x000b0:
		/*
		 *  If this is not implemented, the IP30 PROM complains during
		 *  bootup:
		 *
		 *           *FAILED*
		 *    Address: 0xffffffffbf6000b0, Expected:
		 *        0x0000000000000001, Received: 0x0000000000000000
		 */
		if (writeflag == MEM_WRITE) {
			d->reg_0x000b0 = idata;
		} else {
			odata = d->reg_0x000b0;
		}
		break;

	default:
		if (writeflag == MEM_WRITE) {
			debug("[ sgi_ip30_4: unimplemented write to address"
			    " 0x%x, data=0x%02x ]\n",
			    (int)relative_addr, (int)idata);
		} else {
			debug("[ sgi_ip30_4: unimplemented read from address"
			    " 0x%x ]\n", (int)relative_addr);
		}
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVICE_ACCESS(sgi_ip30_5)
{
	struct sgi_ip30_data *d = (struct sgi_ip30_data *) extra;
	uint64_t idata = 0, odata = 0;

	idata = memory_readmax64(cpu, data, len);

	switch (relative_addr) {

	case 0x00000:
		if (writeflag == MEM_WRITE) {
			d->reg_0x00000 = idata;
		} else {
			odata = d->reg_0x00000;
		}
		break;

	default:
		if (writeflag == MEM_WRITE) {
			debug("[ sgi_ip30_5: unimplemented write to address "
			    "0x%x, data=0x%02x ]\n", (int)relative_addr,
			    (int)idata);
		} else {
			debug("[ sgi_ip30_5: unimplemented read from address "
			    "0x%x ]\n", (int)relative_addr);
		}
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVINIT(sgi_ip30)
{
	struct sgi_ip30_data *d;

	CHECK_ALLOCATION(d = (struct sgi_ip30_data *) malloc(sizeof(struct sgi_ip30_data)));
	memset(d, 0, sizeof(struct sgi_ip30_data));

	memory_device_register(devinit->machine->memory, "sgi_ip30_1",
	    devinit->addr, DEV_SGI_IP30_LENGTH, dev_sgi_ip30_access, (void *)d,
	    DM_DEFAULT, NULL);
	memory_device_register(devinit->machine->memory, "sgi_ip30_2", 
	    0x10000000, 0x10000, dev_sgi_ip30_2_access, (void *)d, DM_DEFAULT,
	    NULL);
	memory_device_register(devinit->machine->memory, "sgi_ip30_3",
	    0x1f000000, 0x10000, dev_sgi_ip30_3_access, (void *)d, DM_DEFAULT,
	    NULL);
	memory_device_register(devinit->machine->memory, "sgi_ip30_4",
	    0x1f600000, 0x10000, dev_sgi_ip30_4_access, (void *)d, DM_DEFAULT,
	    NULL);
	memory_device_register(devinit->machine->memory, "sgi_ip30_5",
	    0x1f6c0000, 0x10000, dev_sgi_ip30_5_access, (void *)d, DM_DEFAULT,
	    NULL);

	machine_add_tickfunction(devinit->machine,
	    dev_sgi_ip30_tick, d, 16);

	return 1;
}

