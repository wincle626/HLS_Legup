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
 *  COMMENT: RICOH RS5C313 Real Time Clock
 *
 *  The RS5C313 has 16 registers, see rs5c313reg.h for details. These registers
 *  are addressed at byte offsets.
 *
 *  Note: The only use for this device so far is in the Landisk, connected to
 *        the SH4 SCI pins. In the Landisk machine, the RS5C313 is placed at
 *        a fake (high) address in memory.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "cpu.h"
#include "device.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"

#include "thirdparty/rs5c313reg.h"


#define	DEV_RS5C313_LENGTH	16

struct rs5c313_data {
	uint8_t		reg[DEV_RS5C313_LENGTH];
};


/*
 *  rs5c313_update_time():
 *
 *  Set the RS5C313 registers to correspond to the host's clock.
 */
static void rs5c313_update_time(struct rs5c313_data *d)
{
	struct tm *tmp;
	time_t timet;

	timet = time(NULL);
	tmp = gmtime(&timet);

	d->reg[RS5C313_SEC1]   = tmp->tm_sec % 10;
	d->reg[RS5C313_SEC10]  = tmp->tm_sec / 10;
	d->reg[RS5C313_MIN1]   = tmp->tm_min % 10;
	d->reg[RS5C313_MIN10]  = tmp->tm_min / 10;
	d->reg[RS5C313_HOUR1]  = tmp->tm_hour % 10;
	d->reg[RS5C313_HOUR10] = tmp->tm_hour / 10;

	/*  WDAY. Zero-based. TODO: Is this correct?  */
	d->reg[RS5C313_WDAY]   = tmp->tm_wday;

	d->reg[RS5C313_DAY1]   = tmp->tm_mday % 10;
	d->reg[RS5C313_DAY10]  = tmp->tm_mday / 10;
	d->reg[RS5C313_MON1]   = (tmp->tm_mon + 1) % 10;
	d->reg[RS5C313_MON10]  = (tmp->tm_mon + 1) / 10;
	d->reg[RS5C313_YEAR1]  = tmp->tm_year % 10;
	d->reg[RS5C313_YEAR10] = (tmp->tm_year / 10) % 10;
}


DEVICE_ACCESS(rs5c313)
{
	struct rs5c313_data *d = (struct rs5c313_data *) extra;
	uint64_t idata = 0, odata = 0;

	rs5c313_update_time(d);

	/*  Generic register read/write:  */
	if (writeflag == MEM_WRITE) {
		idata = memory_readmax64(cpu, data, len);
		d->reg[relative_addr] = idata & 0x0f;
	} else {
		odata = d->reg[relative_addr] & 0x0f;
		memory_writemax64(cpu, data, len, odata);
	}

	return 1;
}


DEVINIT(rs5c313)
{
	struct rs5c313_data *d;

	CHECK_ALLOCATION(d = (struct rs5c313_data *) malloc(sizeof(struct rs5c313_data)));
	memset(d, 0, sizeof(struct rs5c313_data));

	/*  Default values:  */
	d->reg[RS5C313_CTRL] = CTRL_24H;

	memory_device_register(devinit->machine->memory, devinit->name,
	    devinit->addr, DEV_RS5C313_LENGTH,
	    dev_rs5c313_access, (void *)d, DM_DEFAULT, NULL);

	return 1;
}

