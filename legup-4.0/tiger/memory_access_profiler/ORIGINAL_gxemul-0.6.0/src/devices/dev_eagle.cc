/*
 *  Copyright (C) 2003-2009  Anders Gavare.  All rights reserved.
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
 *  COMMENT: Motorola MPC105 "Eagle" host bridge
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bus_isa.h"
#include "bus_pci.h"
#include "cpu.h"
#include "device.h"
#include "interrupt.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"


struct eagle_data {
	struct interrupt irq;

	struct pci_data	*pci_data;
};


DEVICE_ACCESS(eagle)
{
	struct eagle_data *d = (struct eagle_data *) extra;
	uint64_t idata = 0, odata = 0;
	int bus, dev, func, reg;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len|MEM_PCI_LITTLE_ENDIAN);

	/*
	 *  Pass accesses to ISA ports 0xcf8 and 0xcfc onto bus_pci_*:
	 */

	switch (relative_addr) {

	case 0:	/*  Address:  */
		bus_pci_decompose_1(idata, &bus, &dev, &func, &reg);
		bus_pci_setaddr(cpu, d->pci_data, bus, dev, func, reg);
		break;

	case 4:	/*  Data:  */
		bus_pci_data_access(cpu, d->pci_data, writeflag == MEM_READ?
		    &odata : &idata, len, writeflag);
		break;
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len|MEM_PCI_LITTLE_ENDIAN, odata);

	return 1;
}


DEVINIT(eagle)
{
	struct eagle_data *d;
	uint64_t pci_io_offset, pci_mem_offset;
	uint64_t isa_portbase = 0, isa_membase = 0;
	uint64_t pci_portbase = 0, pci_membase = 0;
	char pci_irq_base[300];
	char isa_irq_base[300];

	CHECK_ALLOCATION(d = (struct eagle_data *) malloc(sizeof(struct eagle_data)));
	memset(d, 0, sizeof(struct eagle_data));

	/*  The interrupt path to the CPU at which we are connected:  */
	INTERRUPT_CONNECT(devinit->interrupt_path, d->irq);

	/*
	 *  According to http://www.beatjapan.org/mirror/www.be.com/
	 *  aboutbe/benewsletter/Issue27.html#Cookbook :
	 *
	 *  "HARDWARE MEMORY MAP
	 *   The MPC105 defines the physical memory map of the system as
	 *   follows:
	 *
	 *   Start        Size         Description
	 *
	 *   0x00000000   0x40000000   Physical RAM
	 *   0x40000000   0x40000000   Other system memory
	 *                             (motherboard glue regs)
	 *   0x80000000   0x00800000   ISA I/O
	 *   0x81000000   0x3E800000   PCI I/O
	 *   0xBFFFFFF0   0x00000010   PCI/ISA interrupt acknowledge
	 *   0xC0000000   0x3F000000   PCI memory
	 *   0xFF000000   0x01000000   ROM/flash
	 */

	/*  TODO: Make these work like the BE web page stated...  */
	pci_io_offset  = 0x80000000ULL;
	pci_mem_offset = 0xc0000000ULL;
	pci_portbase   = 0x00000000ULL;
	pci_membase    = 0x00000000ULL;
	isa_portbase   = 0x80000000ULL;
	isa_membase    = 0xc0000000ULL;

	switch (devinit->machine->machine_type) {
	/* case MACHINE_BEBOX:
		snprintf(pci_irq_base, sizeof(pci_irq_base), "%s.bebox",
		    devinit->interrupt_path);
		snprintf(isa_irq_base, sizeof(isa_irq_base), "%s.bebox.5",
		    devinit->interrupt_path);
		break; */
	default:
		snprintf(pci_irq_base, sizeof(pci_irq_base), "%s",
		    devinit->interrupt_path);
		snprintf(isa_irq_base, sizeof(isa_irq_base), "%s",
		    devinit->interrupt_path);
	}

	/*  Create a PCI bus:  */
	d->pci_data = bus_pci_init(devinit->machine, devinit->interrupt_path,
	    pci_io_offset, pci_mem_offset,
	    pci_portbase, pci_membase, pci_irq_base,
	    isa_portbase, isa_membase, isa_irq_base);

	/*  Add the PCI glue for the controller itself:  */
	bus_pci_add(devinit->machine, d->pci_data,
	    devinit->machine->memory, 0, 0, 0, "eagle");

	/*  ADDR and DATA configuration ports in ISA space:  */
	memory_device_register(devinit->machine->memory, "eagle",
	    isa_portbase + BUS_PCI_ADDR, 8, dev_eagle_access, d,
	    DM_DEFAULT, NULL);

	switch (devinit->machine->machine_type) {

	/* case MACHINE_BEBOX:
		bus_isa_init(devinit->machine, isa_irq_base,
		    BUS_ISA_IDE0 | BUS_ISA_VGA, isa_portbase, isa_membase);
		bus_pci_add(devinit->machine, d->pci_data,
		    devinit->machine->memory, 0, 11, 0, "i82378zb");
		break; */

	case MACHINE_PREP:
		bus_pci_add(devinit->machine, d->pci_data,
		    devinit->machine->memory, 0, 11, 0, "ibm_isa");
		break;

	case MACHINE_MVMEPPC:
		bus_isa_init(devinit->machine, isa_irq_base,
		    BUS_ISA_LPTBASE_3BC, isa_portbase, isa_membase);

		switch (devinit->machine->machine_subtype) {
		case MACHINE_MVMEPPC_1600:
			bus_pci_add(devinit->machine, d->pci_data,
			    devinit->machine->memory, 0, 11, 0, "i82378zb");
			break;
		default:fatal("unimplemented machine subtype for "
			    "eagle/mvmeppc\n");
			exit(1);
		}
		break;

	default:fatal("unimplemented machine type for eagle\n");
		exit(1);
	}

	devinit->return_ptr = d->pci_data;

	return 1;
}

