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
 *  COMMENT: M88200/M88204 CMMU (Cache/Memory Management Unit)
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

#include "thirdparty/m8820x.h"
#include "thirdparty/m8820x_pte.h"


struct m8820x_data {
	int		cmmu_nr;
};


/*
 *  m8820x_command():
 *
 *  Handle M8820x commands written to the System Command Register.
 */
static void m8820x_command(struct cpu *cpu, struct m8820x_data *d)
{
	uint32_t *regs = cpu->cd.m88k.cmmu[d->cmmu_nr]->reg;
	int cmd = regs[CMMU_SCR];
	uint32_t sar = regs[CMMU_SAR];
	size_t i;
	uint32_t super, all;

	switch (cmd) {

	case CMMU_FLUSH_CACHE_CB_LINE:
	case CMMU_FLUSH_CACHE_CB_PAGE:
	case CMMU_FLUSH_CACHE_INV_LINE:
	case CMMU_FLUSH_CACHE_INV_PAGE:
	case CMMU_FLUSH_CACHE_INV_ALL:
	case CMMU_FLUSH_CACHE_CBI_LINE:
	case CMMU_FLUSH_CACHE_CBI_PAGE:
	case CMMU_FLUSH_CACHE_CBI_SEGMENT:
	case CMMU_FLUSH_CACHE_CBI_ALL:
		/*  TODO  */
		break;

	case CMMU_FLUSH_USER_ALL:
	case CMMU_FLUSH_USER_PAGE:
	case CMMU_FLUSH_SUPER_ALL:
	case CMMU_FLUSH_SUPER_PAGE:
		/*  TODO: Segment invalidation.  */

		all = super = 0;
		if (cmd == CMMU_FLUSH_USER_ALL ||
		    cmd == CMMU_FLUSH_SUPER_ALL)
			all = 1;
		if (cmd == CMMU_FLUSH_SUPER_ALL ||
		    cmd == CMMU_FLUSH_SUPER_PAGE)
			super = M8820X_PATC_SUPERVISOR_BIT;

		/*  TODO: Don't invalidate EVERYTHING like this!  */
		cpu->invalidate_translation_caches(cpu, 0, INVALIDATE_ALL);

		for (i=0; i<N_M88200_PATC_ENTRIES; i++) {
			uint32_t v = cpu->cd.m88k.cmmu[d->cmmu_nr]
			    ->patc_v_and_control[i];
			uint32_t p = cpu->cd.m88k.cmmu[d->cmmu_nr]
			    ->patc_p_and_supervisorbit[i];

			/*  Already invalid? Then skip this entry.  */
			if (!(v & PG_V))
				continue;

			/*  Super/user mismatch? Then skip the entry.  */
			if ((p & M8820X_PATC_SUPERVISOR_BIT) != super)
				continue;

			/*  If not all pages are to be invalidated, there
			    must be a virtual address match:  */
			if (!all && (sar & 0xfffff000) != (v & 0xfffff000))
				continue;

			/*  Finally, invalidate the entry:  */
			cpu->cd.m88k.cmmu[d->cmmu_nr]->patc_v_and_control[i]
			    = v & ~PG_V;
		}

		break;

	default:
		fatal("[ m8820x_command: FATAL ERROR! unimplemented "
		    "command 0x%02x ]\n", cmd);
		exit(1);
	}
}


DEVICE_ACCESS(m8820x)
{
	uint64_t idata = 0, odata = 0;
	struct m8820x_data *d = (struct m8820x_data *) extra;
	uint32_t *regs = cpu->cd.m88k.cmmu[d->cmmu_nr]->reg;
	uint32_t *batc = cpu->cd.m88k.cmmu[d->cmmu_nr]->batc;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	if (writeflag == MEM_READ)
		odata = regs[relative_addr / sizeof(uint32_t)];

	switch (relative_addr / sizeof(uint32_t)) {

	case CMMU_IDR:
		if (writeflag == MEM_WRITE) {
			fatal("m8820x: write to CMMU_IDR: TODO\n");
			exit(1);
		}
		break;

	case CMMU_SCR:
		if (writeflag == MEM_READ) {
			fatal("m8820x: read from CMMU_SCR: TODO\n");
			exit(1);
		} else {
			regs[relative_addr / sizeof(uint32_t)] = idata;
			m8820x_command(cpu, d);
		}
		break;

	case CMMU_SSR:
		if (writeflag == MEM_WRITE) {
			fatal("m8820x: write to CMMU_SSR: TODO\n");
			exit(1);
		}
		break;

	case CMMU_PFSR:
	case CMMU_PFAR:
	case CMMU_SAR:
	case CMMU_SCTR:
	case CMMU_SAPR:		/*  TODO: Invalidate something for  */
	case CMMU_UAPR:		/*  SAPR and UAPR writes?  */
		/*  TODO: Don't invalidate everything.  */
		cpu->invalidate_translation_caches(cpu, 0, INVALIDATE_ALL);
		if (writeflag == MEM_WRITE)
			regs[relative_addr / sizeof(uint32_t)] = idata;
		break;

	case CMMU_BWP0:
	case CMMU_BWP1:
	case CMMU_BWP2:
	case CMMU_BWP3:
	case CMMU_BWP4:
	case CMMU_BWP5:
	case CMMU_BWP6:
	case CMMU_BWP7:
		if (writeflag == MEM_WRITE) {
			uint32_t old;
			int i = (relative_addr / sizeof(uint32_t)) - CMMU_BWP0;

			regs[relative_addr / sizeof(uint32_t)] = idata;

			/*  Also write to the specific batc registers:  */
			old = batc[i];
			batc[i] = idata;
			if (old != idata) {
				/*  TODO: Don't invalidate everything?  */
				cpu->invalidate_translation_caches(
				    cpu, 0, INVALIDATE_ALL);
			}
		}
		break;

	case CMMU_CSSP0:
		/*  TODO: Actually care about cache details.  */
		break;

	default:fatal("[ m8820x: unimplemented %s offset 0x%x",
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


DEVINIT(m8820x)
{
	struct m8820x_data *d;

	CHECK_ALLOCATION(d = (struct m8820x_data *) malloc(sizeof(struct m8820x_data)));
	memset(d, 0, sizeof(struct m8820x_data));

	d->cmmu_nr = devinit->addr2;

	memory_device_register(devinit->machine->memory, devinit->name,
	    devinit->addr, M8820X_LENGTH, dev_m8820x_access, (void *)d,
	    DM_DEFAULT, NULL);

	return 1;
}

