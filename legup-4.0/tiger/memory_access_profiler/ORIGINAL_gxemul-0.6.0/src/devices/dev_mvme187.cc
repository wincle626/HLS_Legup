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
 *  COMMENT: MVME187-specific devices and control registers
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

#include "thirdparty/mvme187.h"
#include "thirdparty/mvme_memcreg.h"
#include "thirdparty/m8820x.h"


struct mvme187_data {
	struct memcreg		memcreg;
};


DEVICE_ACCESS(mvme187_memc)
{
	uint64_t idata = 0, odata = 0;
	struct mvme187_data *d = (struct mvme187_data *) extra;
	int controller = 0;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	if (relative_addr & 0x100) {
		controller = 1;
		relative_addr &= ~0x100;
	}

	odata = ((uint8_t*)&d->memcreg)[relative_addr];

	switch (relative_addr) {

	case 0x00:	/* chipid */
	case 0x04:	/* chiprev */
		break;

	case 0x08:	/*  memconf  */
		if (writeflag == MEM_WRITE) {
			fatal("mvme187_memc: Write to relative_addr %i not yet"
			    " implemented!\n");
			exit(1);
		}
		break;

	default:fatal("[ mvme187_memc: unimplemented %s offset 0x%x",
		    writeflag == MEM_WRITE? "write to" : "read from",
		    (int) relative_addr);
		if (writeflag == MEM_WRITE)
			fatal(": 0x%x", (int)idata);
		fatal(" ]\n");
		exit(1);
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVINIT(mvme187)
{
	struct mvme187_data *d;
	char tmpstr[300];
	struct m8820x_cmmu *cmmu;
	int size_per_memc, r;

	CHECK_ALLOCATION(d = (struct mvme187_data *) malloc(sizeof(struct mvme187_data)));
	memset(d, 0, sizeof(struct mvme187_data));


	/*
	 *  Two memory controllers per MVME187 machine:
	 */

	size_per_memc = devinit->machine->physical_ram_in_mb / 2 * 1048576;
	for (r=0; ; r++) {
		if (MEMC_MEMCONF_RTOB(r) > size_per_memc) {
			r--;
			break;
		}
	}

	d->memcreg.memc_chipid = MEMC_CHIPID;
	d->memcreg.memc_chiprev = 1;
	d->memcreg.memc_memconf = r;

	memory_device_register(devinit->machine->memory, devinit->name,
	    MVME187_MEM_CTLR, 0x200, dev_mvme187_memc_access, (void *)d,
	    DM_DEFAULT, NULL);


	/*  Instruction CMMU:  */
	CHECK_ALLOCATION(cmmu = (struct m8820x_cmmu *) malloc(sizeof(struct m8820x_cmmu)));
	memset(cmmu, 0, sizeof(struct m8820x_cmmu));

	devinit->machine->cpus[devinit->machine->bootstrap_cpu]->
	    cd.m88k.cmmu[0] = cmmu;
	/*  This is a 88200, revision 9:  */
	cmmu->reg[CMMU_IDR] = (M88200_ID << 21) | (9 << 16);
	snprintf(tmpstr, sizeof(tmpstr),
	    "m8820x addr=0x%x addr2=0", MVME187_SBC_CMMU_I);
	device_add(devinit->machine, tmpstr);

	/*  ... and data CMMU:  */
	CHECK_ALLOCATION(cmmu = (struct m8820x_cmmu *) malloc(sizeof(struct m8820x_cmmu)));
	memset(cmmu, 0, sizeof(struct m8820x_cmmu));

	devinit->machine->cpus[devinit->machine->bootstrap_cpu]->
	    cd.m88k.cmmu[1] = cmmu;
	/*  This is also a 88200, revision 9:  */
	cmmu->reg[CMMU_IDR] = (M88200_ID << 21) | (9 << 16);
	cmmu->batc[8] = BATC8;
	cmmu->batc[9] = BATC9;
	snprintf(tmpstr, sizeof(tmpstr),
	    "m8820x addr=0x%x addr2=1", MVME187_SBC_CMMU_D);
	device_add(devinit->machine, tmpstr);


	return 1;
}

