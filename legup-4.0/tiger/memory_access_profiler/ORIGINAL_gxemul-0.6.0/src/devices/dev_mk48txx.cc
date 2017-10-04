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
 *  COMMENT: Mostek MK48Txx Real Time Clock
 *
 *  TODO:
 *	Only the MK48T08 is implemented so far.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "cpu.h"
#include "device.h"
#include "emul.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"

#include "thirdparty/mk48txxreg.h"


#define	MK48TXX_LEN		MK48T08_CLKSZ

#define	BCD(x)	((((x) / 10) << 4) + ((x) % 10))

struct mk48txx_data {
	uint8_t		reg[MK48TXX_LEN];
};


void mk48txx_update_regs(struct mk48txx_data *d)
{
	struct tm *tmp;
	time_t timet;

	timet = time(NULL);
	tmp = gmtime(&timet);

	d->reg[MK48T08_CLKOFF + MK48TXX_ISEC] = BCD(tmp->tm_sec);
	d->reg[MK48T08_CLKOFF + MK48TXX_IMIN] = BCD(tmp->tm_min);
	d->reg[MK48T08_CLKOFF + MK48TXX_IHOUR] = BCD(tmp->tm_hour);
	d->reg[MK48T08_CLKOFF + MK48TXX_IWDAY] = tmp->tm_wday + 1;
	d->reg[MK48T08_CLKOFF + MK48TXX_IDAY] = BCD(tmp->tm_mday);
	d->reg[MK48T08_CLKOFF + MK48TXX_IMON] = BCD(tmp->tm_mon + 1);
	d->reg[MK48T08_CLKOFF + MK48TXX_IYEAR] = BCD(tmp->tm_year % 100);
}


DEVICE_ACCESS(mk48txx)
{
	struct mk48txx_data *d = (struct mk48txx_data *) extra;
	uint64_t idata = 0, odata = 0;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	if (writeflag == MEM_READ)
		odata = d->reg[relative_addr];

	if (relative_addr < MK48T08_CLKOFF ||
	    relative_addr >= MK48T08_CLKOFF + MK48TXX_ISEC) {
		/*  Reads and writes to the RAM part of the mk48txx, or
		    the clock registers, are OK:  */
		if (writeflag == MEM_WRITE)
			d->reg[relative_addr] = idata;
		goto ret;
	}

	switch (relative_addr) {

	case MK48T08_CLKOFF + MK48TXX_ICSR:
		if (writeflag == MEM_WRITE) {
			if ((idata & MK48TXX_CSR_READ) &&
			    !(d->reg[relative_addr] & MK48TXX_CSR_READ)) {
				/*  Switching the read bit from 0 to 1 causes
				    registers to be "froozen". In the emulator,
				    simply updating them with data from the
				    host should be good enough.  */
				mk48txx_update_regs(d);
			}
			d->reg[relative_addr] = idata;
		}
		break;

	default:if (writeflag == MEM_READ)
			fatal("[ mk48txx: unimplemented READ from offset 0x%x ]"
			    "\n", (int)relative_addr);
		else
			fatal("[ mk48txx: unimplemented WRITE to offset 0x%x: "
			    "0x%x ]\n", (int)relative_addr, (int)idata);
		exit(1);
	}

ret:
	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVINIT(mk48txx)
{
	struct mk48txx_data *d;

	CHECK_ALLOCATION(d = (struct mk48txx_data *) malloc(sizeof(struct mk48txx_data)));
	memset(d, 0, sizeof(struct mk48txx_data));

	mk48txx_update_regs(d);

	memory_device_register(devinit->machine->memory, devinit->name,
	    devinit->addr, MK48TXX_LEN, dev_mk48txx_access, (void *)d,
	    DM_DEFAULT, NULL);

	return 1;
}

