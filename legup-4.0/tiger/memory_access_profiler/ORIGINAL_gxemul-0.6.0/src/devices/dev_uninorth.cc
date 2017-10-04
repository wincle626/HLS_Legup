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
 *  COMMENT: Uni-North PCI controller (used by MacPPC machines)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bus_pci.h"
#include "cpu.h"
#include "devices.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"


struct uninorth_data {
	int		pciirq;

	struct pci_data	*pci_data;
	uint64_t	cur_addr;
};


DEVICE_ACCESS(uninorth_addr)
{
	struct uninorth_data *d = (struct uninorth_data *) extra;

	if (writeflag == MEM_WRITE) {
		uint64_t idata = memory_readmax64(cpu, data, len
		    | MEM_PCI_LITTLE_ENDIAN);
		int bus, dev, func, reg;

		d->cur_addr = idata;
		if (idata == 0)
			return 0;

		/*  Decompose the Uni-North tag:  */
		if (idata & 1) {
			idata &= ~1;
			bus_pci_decompose_1(idata, &bus, &dev, &func, &reg);
		} else {
			bus = 0;
			for (dev=11; dev<32; dev++)
				if (idata & (1 << dev))
					break;
			if (dev == 32)
				fatal("[ dev_uninorth_addr_access: no dev? "
				    "idata=0x%08x ]\n", (int)idata);

			func = (idata >> 8) & 7;
			reg = idata & 0xff;
		}

		bus_pci_setaddr(cpu, d->pci_data, bus, dev, func, reg);
	} else {
		/*  TODO: is returning the current address like this
		    the correct behaviour?  */
		memory_writemax64(cpu, data, len | MEM_PCI_LITTLE_ENDIAN,
		    d->cur_addr);
	}

	return 1;
}


DEVICE_ACCESS(uninorth_data)
{
	struct uninorth_data *d = (struct uninorth_data *) extra;
	uint64_t idata = 0, odata = 0;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len|MEM_PCI_LITTLE_ENDIAN);

	bus_pci_data_access(cpu, d->pci_data, writeflag == MEM_READ? &odata :
	    &idata, len, writeflag);

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len|MEM_PCI_LITTLE_ENDIAN, odata);

	return 1;
}


struct pci_data *dev_uninorth_init(struct machine *machine, struct memory *mem,
	uint64_t addr, int isa_irqbase, int pciirq)
{
	struct uninorth_data *d;
	uint64_t pci_io_offset, pci_mem_offset;
	uint64_t isa_portbase = 0, isa_membase = 0;
	uint64_t pci_portbase = 0, pci_membase = 0;

	CHECK_ALLOCATION(d = (struct uninorth_data *) malloc(sizeof(struct uninorth_data)));
	memset(d, 0, sizeof(struct uninorth_data));

	d->pciirq = pciirq;

	pci_io_offset  = 0x00000000ULL;
	pci_mem_offset = 0x00000000ULL;
	pci_portbase   = 0xd0000000ULL;
	pci_membase    = 0xd1000000ULL;
	isa_portbase   = 0xd2000000ULL;
	isa_membase    = 0xd3000000ULL;

	/*  Create a PCI bus:  */
	d->pci_data = bus_pci_init(machine, "ZZZ_irq_stuff",
	    pci_io_offset, pci_mem_offset,
	    pci_portbase, pci_membase, "XXX_pci_irqbase",
	    isa_portbase, isa_membase, "YYY_isa_irqbase");

	/*  Add the PCI glue for the controller itself:  */
	bus_pci_add(machine, d->pci_data, mem, 0, 11, 0, "uninorth");

	/*  ADDR and DATA configuration ports:  */
	memory_device_register(mem, "uninorth_pci_addr", addr + 0x800000,
	    4, dev_uninorth_addr_access, d, DM_DEFAULT, NULL);
	memory_device_register(mem, "uninorth_pci_data", addr + 0xc00000,
	    8, dev_uninorth_data_access, d, DM_DEFAULT, NULL);

	return d->pci_data;
}

