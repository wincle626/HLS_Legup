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
 *  COMMENT: SGI IP22 stuff
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "devices.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"

#include "thirdparty/imcreg.h"


#define	SGI_IP22_TICK_SHIFT		14


DEVICE_TICK(sgi_ip22)
{
	struct sgi_ip22_data *d = (struct sgi_ip22_data *) extra;

	// TODO: Document this!
	if (d->reg[0x38 / 4] != 0)
		d->reg[0x38 / 4] --;
}


/*
 *  dev_sgi_ip22_imc_access():
 *
 *  The memory controller (?).
 */
DEVICE_ACCESS(sgi_ip22_imc)
{
	struct sgi_ip22_data *d = (struct sgi_ip22_data *) extra;
	uint64_t idata = 0, odata = 0;
	int regnr;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	regnr = relative_addr / sizeof(uint32_t);

	if (writeflag == MEM_WRITE)
		d->imc_reg[regnr] = idata;
	else
		odata = d->imc_reg[regnr];

	switch (relative_addr) {
	case (IMC_CPUCTRL0 - IP22_IMC_BASE):
		if (writeflag == MEM_WRITE) {
			/*  debug("[ sgi_ip22_imc: write to "
			    "IMC_CPUCTRL0, data=0x%08x ]\n", (int)idata);  */
		} else {
			/*  debug("[ sgi_ip22_imc: read from IMC_CPUCTRL0, "
			    "data=0x%08x ]\n", (int)odata);  */
		}
		break;
	case (IMC_SYSID - IP22_IMC_BASE):
		if (writeflag == MEM_WRITE) {
			debug("[ sgi_ip22_imc: unimplemented write "
			    "IMC_SYSID, data=0x%08x ]\n", (int)idata);
		} else {
			/*  Lowest 4 bits are the revision bits.  */
			odata = 3;  /*  + IMC_SYSID_HAVEISA;  */
			/*  debug("[ sgi_ip22_imc: read from IMC_SYSID, "
			    "data=0x%08x ]\n", (int)odata);  */
		}
		break;
	case (IMC_WDOG - IP22_IMC_BASE):
		if (writeflag == MEM_WRITE) {
			/*  debug("[ sgi_ip22_imc: write to IMC_WDOG, "
			    "data=0x%08x ]\n", (int)idata);  */
		} else {
			/*  debug("[ sgi_ip22_imc: read from IMC_WDOG, "
			    "data=0x%08x ]\n", (int)odata);  */
		}
		break;
	case (IMC_MEMCFG0 - IP22_IMC_BASE):
		if (writeflag == MEM_WRITE) {
			debug("[ sgi_ip22_imc: unimplemented write "
			    "IMC_MEMCFG0, data=0x%08x ]\n", (int)idata);
		} else {
			odata = 0x3100 + (0x8000000 >> 22);  /*  ? TODO  */
			/*  debug("[ sgi_ip22_imc: read from IMC_MEMCFG0,"
			    " data=0x%08x ]\n", (int)odata);  */
		}
		break;
	case (IMC_MEMCFG1 - IP22_IMC_BASE):
		if (writeflag == MEM_WRITE) {
			debug("[ sgi_ip22_imc: unimplemented write "
			    "IMC_MEMCFG1, data=0x%08x ]\n", (int)idata);
		} else {
			odata = 0;
			/*  debug("[ sgi_ip22_imc: read from IMC_MEMCFG1, "
			    "data=0x%08x ]\n", (int)odata);  */
		}
		break;
	case (IMC_EEPROM - IP22_IMC_BASE):
		/*
		 *  The IP22 prom tries to access this during bootup,
		 *  but I have no idea how it works.
		 */
		if (writeflag == MEM_WRITE) {
			debug("[ sgi_ip22_imc: write to IMC_EEPROM, data="
			    "0x%08x ]\n", (int)idata);
		} else {
			odata = random() & 0x1e;
			debug("[ sgi_ip22_imc: read from IMC_WDOG, "
			    "data=0x%08x ]\n", (int)odata);
		}
		break;
	default:
		if (writeflag == MEM_WRITE) {
			debug("[ sgi_ip22_imc: unimplemented write to "
			    "address 0x%x, data=0x%08x ]\n",
			    (int)relative_addr, (int)idata);
		} else {
			debug("[ sgi_ip22_imc: unimplemented read from "
			    "address 0x%x, data=0x%08x ]\n",
			    (int)relative_addr, (int)odata);
		}
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


/*
 *  dev_sgi_ip22_unknown_access():
 *
 *  A so far unknown device, used by the IP22 prom during startup.
 */
DEVICE_ACCESS(sgi_ip22_unknown)
{
	struct sgi_ip22_data *d = (struct sgi_ip22_data *) extra;
	uint64_t idata = 0, odata = 0;

	idata = memory_readmax64(cpu, data, len);

	switch (relative_addr) {
	case 0x04:
		if (writeflag == MEM_WRITE) {
			debug("[ sgi_ip22_unknown: write to address 0x%x,"
			    " data=0x%08x ]\n", (int)relative_addr, (int)idata);
		} else {
			odata = d->unknown_timer;
			d->unknown_timer += 100;
			debug("[ sgi_ip22_unknown: read from address 0x%x, "
			    "data=0x%08x ]\n", (int)relative_addr, (int)odata);
		}
		break;
	default:
		if (writeflag == MEM_WRITE) {
			debug("[ sgi_ip22_unknown: unimplemented write to "
			    "address 0x%x, data=0x%08x ]\n",
			    (int)relative_addr, (int)idata);
		} else {
			debug("[ sgi_ip22_unknown: unimplemented read from "
			    "address 0x%x, data=0x%08x ]\n",
			    (int)relative_addr, (int)odata);
		}
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


/*
 *  dev_sgi_ip22_unknown2_access():
 *
 *  A so far unknown device, used by the IP22 prom during startup.
 */
DEVICE_ACCESS(sgi_ip22_unknown2)
{
	struct sgi_ip22_data *d = (struct sgi_ip22_data *) extra;
	uint64_t idata = 0, odata = 0;
	int regnr;

	idata = memory_readmax64(cpu, data, len);
	regnr = relative_addr / sizeof(uint32_t);

	if (writeflag == MEM_WRITE)
		d->unknown2_reg[regnr] = idata;
	else
		odata = d->unknown2_reg[regnr];

	switch (relative_addr) {
	default:
		if (writeflag == MEM_WRITE) {
			debug("[ sgi_ip22_unknown2: unimplemented write "
			    "to address 0x%x, data=0x%08x ]\n",
			    (int)relative_addr, (int)idata);
		} else {
			debug("[ sgi_ip22_unknown2: unimplemented read from "
			    "address 0x%x, data=0x%08x ]\n",
			    (int)relative_addr, (int)odata);
		}
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVICE_ACCESS(sgi_ip22_sysid)
{
	struct sgi_ip22_data *d = (struct sgi_ip22_data *) extra;
	uint64_t idata = 0, odata = 0;

	idata = memory_readmax64(cpu, data, len);

	if (writeflag == MEM_WRITE) {
		debug("[ sgi_ip22_sysid: write to address 0x%x, "
		    "data=0x%08x ]\n", (int)relative_addr, (int)idata);
	} else {
		/*
		 *  According to NetBSD's sgimips/ip22.c:
		 *
		 *  printf("IOC rev %d, machine %s, board rev %d\n",
		 *	(sysid >> 5) & 0x07,
		 *	(sysid & 1) ? "Indigo2 (Fullhouse)" : "Indy (Guiness)",
		 *	(sysid >> 1) & 0x0f);
		 */

		/*  IOC rev 1, Guiness, board rev 3:  */
		odata = (1 << 5) + (3 << 1) + (d->guiness_flag? 0 : 1);

		debug("[ sgi_ip22_sysid: read from address 0x%x, data="
		    "0x%08x ]\n", (int)relative_addr, (int)odata);
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVICE_ACCESS(sgi_ip22)
{
	struct sgi_ip22_data *d = (struct sgi_ip22_data *) extra;
	uint64_t idata = 0, odata = 0;
	int regnr;

	idata = memory_readmax64(cpu, data, len);
	regnr = relative_addr / sizeof(uint32_t);

	if (writeflag == MEM_WRITE)
		d->reg[regnr] = idata;
	else
		odata = d->reg[regnr];

	/*  Read from/write to the sgi_ip22:  */
	switch (relative_addr) {
	case 0x00:	/*  local0 irq stat  */
		if (writeflag == MEM_WRITE) {
			debug("[ sgi_ip22: write to local0 IRQ STAT, "
			    "data=0x%llx ]\n", (long long)idata);
		} else {
			debug("[ sgi_ip22: read from local0 IRQ STAT, "
			    "data=0x%llx ]\n", (long long)odata);
		}
		break;
	case 0x04:	/*  local0 irq mask  */
		if (writeflag == MEM_WRITE) {
			/*
			 *  Ugly hack: if an interrupt is asserted, and someone
			 *  writes to this mask register, the interrupt should
			 *  be masked. That is, sgi_ip22_interrupt() in
			 *  src/machine.c has to be called to deal with this.
			 *  The ugly solution I choose here is to deassert
			 *  some interrupt which should never be used anyway.
			 *  (TODO: Fix this.)
			 */

fatal("TODO: ip22 legacy interrupt rewrite!\n");
abort();

//			cpu_interrupt_ack(cpu, 8 + 63);
//			debug("[ sgi_ip22: write to local0 IRQ MASK, "
//			    "data=0x%llx ]\n", (long long)idata);
		} else {
			debug("[ sgi_ip22: read from local0 IRQ MASK, "
			    "data=0x%llx ]\n", (long long)odata);
		}
		break;
	case 0x08:	/*  local1 irq stat  */
		if (writeflag == MEM_WRITE) {
			debug("[ sgi_ip22: write to local1 IRQ STAT, "
			    "data=0x%llx ]\n", (long long)idata);
		} else {
			debug("[ sgi_ip22: read from local1 IRQ STAT, "
			    "data=0x%llx ]\n", (long long)odata);
		}
		break;
	case 0x0c:	/*  local1 irq mask  */
		if (writeflag == MEM_WRITE) {
			/*  See commen above, about local0 irq mask.  */

fatal("TODO: ip22 legacy interrupt rewrite!\n");
abort();
//			cpu_interrupt_ack(cpu, 8 + 63);
//			debug("[ sgi_ip22: write to local1 IRQ MASK, "
//			    "data=0x%llx ]\n", (long long)idata);
		} else {
			debug("[ sgi_ip22: read from local1 IRQ MASK, "
			    "data=0x%llx ]\n", (long long)odata);
		}
		break;
	case 0x10:
		if (writeflag == MEM_WRITE) {
			debug("[ sgi_ip22: write to mappable IRQ STAT, "
			    "data=0x%llx ]\n", (long long)idata);
		} else {
			debug("[ sgi_ip22: read from mappable IRQ STAT, "
			    "data=0x%llx ]\n", (long long)odata);
		}
		break;
	case 0x14:
		if (writeflag == MEM_WRITE) {
			debug("[ sgi_ip22: write to mappable local0 IRQ "
			    "MASK, data=0x%llx ]\n", (long long)idata);
		} else {
			debug("[ sgi_ip22: read from mappable local0 IRQ "
			    "MASK, data=0x%llx ]\n", (long long)odata);
		}
		break;
	case 0x18:
		if (writeflag == MEM_WRITE) {
			debug("[ sgi_ip22: write to mappable local1 IRQ "
			    "MASK, data=0x%llx ]\n", (long long)idata);
		} else {
			debug("[ sgi_ip22: read from mappable local1 IRQ "
			    "MASK, data=0x%llx ]\n", (long long)odata);
		}
		break;
	case 0x38:	/*  timer count  */
		if (writeflag == MEM_WRITE) {
			/*  Two byte values are written to this address,
			    sequentially...  TODO  */
		} else {
			/*  The timer is decreased by the tick function.  */
		}
		break;
	case 0x3b:	/*  ?  */
		odata = random();
		break;
	case 0x3c:	/*  timer control  */
		break;
	case 0x3f:	/*  ?  */
		odata = random();
		break;
	default:
		if (writeflag == MEM_WRITE) {
			debug("[ sgi_ip22: unimplemented write to address "
			    "0x%x, data=0x%02x ]\n", (int)relative_addr,
			    (int)idata);
		} else {
			debug("[ sgi_ip22: unimplemented read from address "
			    "0x%llx ]\n", (long long)relative_addr);
		}
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


struct sgi_ip22_data *dev_sgi_ip22_init(struct machine *machine,
	struct memory *mem, uint64_t baseaddr, int guiness_flag)
{
	struct sgi_ip22_data *d;

	CHECK_ALLOCATION(d = (struct sgi_ip22_data *) malloc(sizeof(struct sgi_ip22_data)));
	memset(d, 0, sizeof(struct sgi_ip22_data));

	d->guiness_flag = guiness_flag;

	memory_device_register(mem, "sgi_ip22", baseaddr, DEV_SGI_IP22_LENGTH,
	    dev_sgi_ip22_access, (void *)d, DM_DEFAULT, NULL);
	memory_device_register(mem, "sgi_ip22_sysid", 0x1fbd9858, 0x8,
	    dev_sgi_ip22_sysid_access, (void *)d, DM_DEFAULT, NULL);
	memory_device_register(mem, "sgi_ip22_imc", IP22_IMC_BASE,
	    DEV_SGI_IP22_IMC_LENGTH, dev_sgi_ip22_imc_access, (void *)d,
	    DM_DEFAULT, NULL);
	memory_device_register(mem, "sgi_ip22_unknown", 0x1fa01000, 0x10,
	    dev_sgi_ip22_unknown_access, (void *)d, DM_DEFAULT, NULL);
	memory_device_register(mem, "sgi_ip22_unknown2", IP22_UNKNOWN2_BASE,
	    DEV_SGI_IP22_UNKNOWN2_LENGTH, dev_sgi_ip22_unknown2_access,
	    (void *)d, DM_DEFAULT, NULL);

	machine_add_tickfunction(machine, dev_sgi_ip22_tick, d,
	    SGI_IP22_TICK_SHIFT);

	return d;
}

