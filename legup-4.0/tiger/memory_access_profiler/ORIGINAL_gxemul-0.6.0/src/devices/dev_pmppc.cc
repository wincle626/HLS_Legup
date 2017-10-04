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
 *  COMMENT: Artesyn PM/PPC motherboard registers
 *
 *  NOTE/TODO: Only enough to boot NetBSD/pmppc has been implemented.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "device.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"

#include "thirdparty/pmppc.h"


struct pmppc_data {
	uint8_t		config0;
	uint8_t		config1;

	uint8_t		reset_reg;
};


DEVICE_ACCESS(pmppc_board)
{
	struct pmppc_data *d = (struct pmppc_data *) extra;
	uint64_t idata = 0, odata = 0;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	relative_addr += PMPPC_CONFIG0;

	switch (relative_addr) {

	case PMPPC_CONFIG0:
		if (writeflag==MEM_READ) {
			odata = d->config0;
		} else {
			debug("[ pmppc: UNIMPLEMENTED write to PMPPC_CONFIG0:"
			    " 0x%02x ]\n", (int)idata);
		}
		break;

	case PMPPC_CONFIG1:
		if (writeflag==MEM_READ) {
			odata = d->config1;
		} else {
			debug("[ pmppc: UNIMPLEMENTED write to PMPPC_CONFIG1:"
			    " 0x%02x ]\n", (int)idata);
		}
		break;

	case PMPPC_RESET:
		if (writeflag==MEM_READ) {
			odata = d->reset_reg;
		} else {
			if (d->reset_reg == PMPPC_RESET_SEQ_STEP1 &&
			    idata == PMPPC_RESET_SEQ_STEP2) {
				cpu->running = 0;
				cpu->machine->
				    exit_without_entering_debugger = 1;
			}
			d->reset_reg = idata;
		}
		break;

	default:
		if (writeflag==MEM_READ) {
			debug("[ pmppc: UNIMPLEMENTED read from 0x%08lx ]\n",
			    (long)relative_addr);
		} else {
			debug("[ pmppc: UNIMPLEMENTED write to  0x%08lx:"
			    " 0x%02x ]\n", (long)relative_addr, (int)idata);
		}
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVINIT(pmppc)
{
	struct pmppc_data *d;
	struct memory *mem = devinit->machine->memory;

	CHECK_ALLOCATION(d = (struct pmppc_data *) malloc(sizeof(struct pmppc_data)));
	memset(d, 0, sizeof(struct pmppc_data));

	/*
	 *  Based on how NetBSD/pmppc's pmppc_setup() works:
	 *
	 *  config0:
	 *	bit 7		Is monarch (?).
	 *	bit 5		Has ethernet.
	 *	bit 4		1 = No RTC.
	 *	bits 3..2	Flash size (00 = 256 MB, 01 = 128 MB,
	 *			10 = 64 MB, 11 = 32 MB).
	 *	bits 1..0	Flash width (00 = 64, 01 = 32, 10 = 16, 11 = 0).
	 *
	 *  config1:
	 *	bit 7		Boot device is FLASH (1) or ROM (0).
	 *	bit 6		Has ECC.
	 *	bits 5..4	Memory size: 00 = 32 MB, 01 = 64 MB,
	 *			10 = 128 MB, 11 = 256 MB.
	 *	bits 3..2	L2 cache.
	 *	bits 1..0	Bus frequency: 00 = 66.66 MHz, 01 = 83.33 MHz,
	 *			10 = 100.00 MHz, 11 = reserved?
	 */
	d->config0 = 0x20;
	d->config1 = 0;

	if (mem->physical_max == 32*1048576) {
	} else if (mem->physical_max == 64*1048576) {
		d->config1 |= 0x01;
	} else if (mem->physical_max == 128*1048576) {
		d->config1 |= 0x10;
	} else if (mem->physical_max == 256*1048576) {
		d->config1 |= 0x11;
	} else {
		fatal("A PM/PPC can have 32, 64, 128, or 256 MB RAM.\n");
		exit(1);
	}

	memory_device_register(mem, "pmppc_board",
	    PMPPC_CONFIG0, 0x10, dev_pmppc_board_access, d, DM_DEFAULT, NULL);

	return 1;
}

