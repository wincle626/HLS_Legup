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
 *  COMMENT: IBM CPC700 bridge (PCI and interrupt controller)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bus_pci.h"
#include "cpu.h"
#include "device.h"
#include "interrupt.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"

#include "thirdparty/cpc700reg.h"


struct cpc700_data {
	struct interrupt ppc_irq;	/*  Connected to the CPU  */

	uint32_t	sr;		/*  Interrupt Status register  */
	uint32_t	er;		/*  Interrupt Enable register  */

	struct pci_data *pci_data;	/*  PCI bus  */
};


void cpc700_interrupt_assert(struct interrupt *interrupt)
{
	struct cpc700_data *d = (struct cpc700_data *) interrupt->extra;
	d->sr |= interrupt->line;
	if (d->sr & d->er)
		INTERRUPT_ASSERT(d->ppc_irq);
}
void cpc700_interrupt_deassert(struct interrupt *interrupt)
{
	struct cpc700_data *d = (struct cpc700_data *) interrupt->extra;
	d->sr &= ~interrupt->line;
	if (!(d->sr & d->er))
		INTERRUPT_DEASSERT(d->ppc_irq);
}


/*
 *  dev_cpc700_pci_access():
 *
 *  Passes PCI indirect addr and data accesses onto bus_pci.
 */
DEVICE_ACCESS(cpc700_pci)
{
	uint64_t idata = 0, odata = 0;
	int bus, dev, func, reg;
	struct cpc700_data *d = (struct cpc700_data *) extra;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len|MEM_PCI_LITTLE_ENDIAN);

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


/*
 *  dev_cpc700_int_access():
 *
 *  The interrupt controller.
 */
DEVICE_ACCESS(cpc700_int)
{
	struct cpc700_data *d = (struct cpc700_data *) extra;
	uint64_t idata = 0, odata = 0;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	switch (relative_addr) {

	case CPC_UIC_SR:
		/*  Status register (cleared by writing ones):  */
		if (writeflag == MEM_READ) {
			odata = d->sr;
		} else {
			d->sr &= ~idata;
			if (!(d->sr & d->er))
				INTERRUPT_DEASSERT(d->ppc_irq);
		}
		break;

	case CPC_UIC_SRS:
		/*  Status register set:  */
		if (writeflag == MEM_READ) {
			fatal("[ cpc700_int: read from CPC_UIC_SRS? ]\n");
			odata = d->sr;
		} else {
			d->sr = idata;
			if (d->sr & d->er)
				INTERRUPT_ASSERT(d->ppc_irq);
			else
				INTERRUPT_DEASSERT(d->ppc_irq);
		}
		break;

	case CPC_UIC_ER:
		/*  Enable register:  */
		if (writeflag == MEM_READ) {
			odata = d->er;
		} else {
			d->er = idata;
			if (d->sr & d->er)
				INTERRUPT_ASSERT(d->ppc_irq);
			else
				INTERRUPT_DEASSERT(d->ppc_irq);
		}
		break;

	case CPC_UIC_MSR:
		/*  Masked status:  */
		if (writeflag == MEM_READ) {
			odata = d->sr & d->er;
		} else {
			fatal("[ cpc700_int: write to CPC_UIC_MSR? ]\n");
		}
		break;

	default:if (writeflag == MEM_WRITE) {
			fatal("[ cpc700_int: unimplemented write to "
			    "offset 0x%x: data=0x%x ]\n", (int)
			    relative_addr, (int)idata);
		} else {
			fatal("[ cpc700_int: unimplemented read from "
			    "offset 0x%x ]\n", (int)relative_addr);
		}
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVINIT(cpc700)
{
	struct cpc700_data *d;
	char tmp[300];
	int i;

	CHECK_ALLOCATION(d = (struct cpc700_data *) malloc(sizeof(struct cpc700_data)));
	memset(d, 0, sizeof(struct cpc700_data));

	/*  Connect to the CPU's interrupt pin:  */
	INTERRUPT_CONNECT(devinit->interrupt_path, d->ppc_irq);

	/*  Register 32 CPC700 interrupts:  */
	for (i=0; i<32; i++) {
		struct interrupt templ;
		char n[300];
		snprintf(n, sizeof(n), "%s.cpc700.%i",
		    devinit->interrupt_path, i);
		memset(&templ, 0, sizeof(templ));
		templ.line = 1 << i;
		templ.name = n;
		templ.extra = d;
		templ.interrupt_assert = cpc700_interrupt_assert;
		templ.interrupt_deassert = cpc700_interrupt_deassert;
		interrupt_handler_register(&templ);
	}

	/*  Register a PCI bus:  */
	snprintf(tmp, sizeof(tmp), "%s.cpc700", devinit->interrupt_path);
	d->pci_data = bus_pci_init(
	    devinit->machine,
	    tmp,		/*  pciirq path  */
	    0,			/*  pci device io offset  */
	    0,			/*  pci device mem offset  */
	    CPC_PCI_IO_BASE,	/*  PCI portbase  */
	    CPC_PCI_MEM_BASE,	/*  PCI membase: TODO  */
	    tmp,		/*  PCI irqbase  */
	    0,			/*  ISA portbase: TODO  */
	    0,			/*  ISA membase: TODO  */
	    tmp);		/*  ISA irqbase  */

	switch (devinit->machine->machine_type) {

	case MACHINE_PMPPC:
		bus_pci_add(devinit->machine, d->pci_data,
		    devinit->machine->memory, 0, 0, 0, "heuricon_pmppc");
		break;

	default:fatal("!\n! WARNING: cpc700 for non-implemented machine"
		    " type\n!\n");
		exit(1);
	}

	/*  PCI configuration registers:  */
	memory_device_register(devinit->machine->memory, "cpc700_pci",
	    CPC_PCICFGADR, 8, dev_cpc700_pci_access, d, DM_DEFAULT, NULL);

	/*  Interrupt controller:  */
	memory_device_register(devinit->machine->memory, "cpc700_int",
	    CPC_UIC_BASE, CPC_UIC_SIZE, dev_cpc700_int_access, d,
	    DM_DEFAULT, NULL);

	/*  Two serial ports:  */
	snprintf(tmp, sizeof(tmp), "ns16550 irq=%s.cpc700.%i addr=0x%llx "
	    "name2=tty0", devinit->interrupt_path, 31 - CPC_IB_UART_0,
	    (long long)CPC_COM0);
	devinit->machine->main_console_handle = (size_t)
	    device_add(devinit->machine, tmp);
	snprintf(tmp, sizeof(tmp), "ns16550 irq=%s.cpc700.%i addr=0x%llx "
	    "name2=tty1", devinit->interrupt_path, 31 - CPC_IB_UART_1,
	    (long long)CPC_COM1);
	device_add(devinit->machine, tmp);

	devinit->return_ptr = d->pci_data;

	return 1;
}

