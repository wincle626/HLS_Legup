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
 *  COMMENT: V3 Semiconductor PCI controller
 *
 *  The ISA interrupt controller part forwards ISA interrupts as follows
 *  (on Algor P5064):
 *
 *	ISA interrupt 3 and 4		-> MIPS interrupt 4 ("Local")
 *	All other ISA interrupts	-> MIPS interrupt 2 ("ISA")
 *
 *  See NetBSD's src/sys/arch/algor/pci/ for details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bus_pci.h"
#include "cpu.h"
#include "device.h"
#include "devices.h"
#include "interrupt.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"


struct v3_data {
	struct interrupt	irq_isa;
	struct interrupt	irq_local;
	uint8_t			secondary_mask1;

	struct pic8259_data* pic1;
	struct pic8259_data* pic2;
	int			*ptr_to_pending_timer_interrupts;

	struct pci_data		*pci_data;
	uint16_t		lb_map0;
};


/*
 *  v3_isa_interrupt_common():
 */
void v3_isa_interrupt_common(struct v3_data *d, int old_isa_assert)
{
	int new_isa_assert;

	/*  Any interrupt assertions on PIC2 go to irq 2 on PIC1  */
	/*  (TODO: don't hardcode this here)  */
	if (d->pic2->irr & ~d->pic2->ier)
		d->pic1->irr |= 0x04;
	else
		d->pic1->irr &= ~0x04;

	new_isa_assert = d->pic1->irr & ~d->pic1->ier;

	if (old_isa_assert == new_isa_assert)
		return;

	if (new_isa_assert & d->secondary_mask1)
		INTERRUPT_ASSERT(d->irq_local);
	else
		INTERRUPT_DEASSERT(d->irq_local);

	if (new_isa_assert & ~d->secondary_mask1)
		INTERRUPT_ASSERT(d->irq_isa);
	else
		INTERRUPT_DEASSERT(d->irq_isa);
}


/*
 *  v3_isa_interrupt_assert():
 *
 *  Called whenever an ISA device asserts an interrupt (0..15).
 *  If the interrupt number is 16, then it is a re-assert.
 */
void v3_isa_interrupt_assert(struct interrupt *interrupt)
{
	struct v3_data *d = (struct v3_data *) interrupt->extra;
	int old_isa_assert, line = interrupt->line;
	int mask = 1 << (line & 7);

	old_isa_assert = d->pic1->irr & ~d->pic1->ier;

	if (line < 8)
		d->pic1->irr |= mask;
	else if (line < 16)
		d->pic2->irr |= mask;

	v3_isa_interrupt_common(d, old_isa_assert);
}


/*
 *  v3_isa_interrupt_deassert():
 *
 *  Called whenever an ISA device deasserts an interrupt (0..15).
 *  If the interrupt number is 16, then it is a re-assert.
 */
void v3_isa_interrupt_deassert(struct interrupt *interrupt)
{
	struct v3_data *d = (struct v3_data *) interrupt->extra;
	int line = interrupt->line, mask = 1 << (line & 7);
	int old_irr1 = d->pic1->irr, old_isa_assert;

	old_isa_assert = old_irr1 & ~d->pic1->ier;

	if (line < 8)
		d->pic1->irr &= ~mask;
	else if (line < 16)
		d->pic2->irr &= ~mask;

	/*  If IRQ 0 has been cleared, then this is a timer interrupt.
	    Let's ack it here:  */
	if (old_irr1 & 1 && !(d->pic1->irr & 1) &&
	    d->ptr_to_pending_timer_interrupts != NULL &&
            (*d->ptr_to_pending_timer_interrupts) > 0)
                (*d->ptr_to_pending_timer_interrupts) --;

	v3_isa_interrupt_common(d, old_isa_assert);
}


DEVICE_ACCESS(v3_pci)
{
	uint64_t idata = 0, odata = 0;
	int bus, dev, func, reg;
	struct v3_data *d = (struct v3_data *) extra;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len|MEM_PCI_LITTLE_ENDIAN);

	/*  Decompose the tag:  */
	relative_addr &= 0xfffff;
	relative_addr |= ((d->lb_map0 & 0xfff0) << 16);
	bus = 0;
	for (dev=24; dev<32; dev++)
		if (relative_addr & (1 << dev))
			break;
	dev -= 24;
	if (dev == 8) {
		fatal("[ v3_pci: NO DEVICE? ]\n");
		dev = 0;
	}
	func = (relative_addr >> 8) & 7;
	reg  = relative_addr & 0xfc;
	bus_pci_setaddr(cpu, d->pci_data, bus, dev, func, reg);

	/*  Pass semi-direct PCI accesses onto bus_pci:  */
	bus_pci_data_access(cpu, d->pci_data, writeflag == MEM_READ?
	    &odata : &idata, len, writeflag);

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len|MEM_PCI_LITTLE_ENDIAN, odata);

	return 1;
}


DEVICE_ACCESS(v3)
{
	struct v3_data *d = (struct v3_data *) extra;
	uint64_t idata = 0, odata = 0;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	switch (relative_addr) {

	case 0x06:	/*  PCI stat  */
		break;

	case 0x08:	/*  Revision  */
		odata = 4;
		break;

	case 0x18:	/*  PCI DMA base 1  */
		odata = 0x11000000;
		break;

	case 0x5e:	/*  LB MAP0  */
		if (writeflag == MEM_READ)
			odata = d->lb_map0;
		else
			d->lb_map0 = idata;
		break;

	case 0x62:	/*  PCI mem base 1  */
		odata = 0x1100;
		break;

	case 0x64:	/*  L2 BASE  */
		odata = 1;	/*  pci i/o enable  */
		break;

	case 0x66:	/*  Map 2  */
		odata = 0x1d00;
		break;

	default:if (writeflag == MEM_WRITE) {
			fatal("[ v3: unimplemented write to "
			    "offset 0x%x: data=0x%x ]\n", (int)
			    relative_addr, (int)idata);
		} else {
			fatal("[ v3: unimplemented read from "
			    "offset 0x%x ]\n", (int)relative_addr);
		}
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVINIT(v3)
{
	struct v3_data *d;
	uint32_t isa_port_base = 0x1d000000;
	char tmpstr[200];
	char isa_irq_base[200];
	char pci_irq_base[200];
	int i;

	CHECK_ALLOCATION(d = (struct v3_data *) malloc(sizeof(struct v3_data)));
	memset(d, 0, sizeof(struct v3_data));

	switch (devinit->machine->machine_type) {
	case MACHINE_ALGOR:
		snprintf(tmpstr, sizeof(tmpstr), "%s.4",
		    devinit->interrupt_path);
		INTERRUPT_CONNECT(tmpstr, d->irq_local);

		snprintf(tmpstr, sizeof(tmpstr), "%s.2",
		    devinit->interrupt_path);
		INTERRUPT_CONNECT(tmpstr, d->irq_isa);

		d->secondary_mask1 = 0x18;
		break;

	default:fatal("!\n! WARNING: v3 for non-implemented machine"
		    " type %i\n!\n", devinit->machine->machine_type);
		exit(1);
	}

	/*
	 *  Register the 16 possible ISA interrupts, plus a dummy. The
	 *  dummy is used by re-asserts.
	 */
	for (i=0; i<17; i++) {
		struct interrupt templ;
		char n[300];
		snprintf(n, sizeof(n), "%s.v3.isa.%i",
		    devinit->interrupt_path, i);
		memset(&templ, 0, sizeof(templ));
		templ.line = i;
		templ.name = n;
		templ.extra = d;
		templ.interrupt_assert = v3_isa_interrupt_assert;
		templ.interrupt_deassert = v3_isa_interrupt_deassert;
		interrupt_handler_register(&templ);
	}

	/*  Register two 8259 PICs:  */
	snprintf(tmpstr, sizeof(tmpstr), "8259 irq=%s.v3.isa.16 addr=0x%llx",
	    devinit->interrupt_path, (long long)(isa_port_base + 0x20));
	d->pic1 = devinit->machine->isa_pic_data.pic1 = (struct pic8259_data *)
	    device_add(devinit->machine, tmpstr);
	d->ptr_to_pending_timer_interrupts =
	    devinit->machine->isa_pic_data.pending_timer_interrupts;

	snprintf(tmpstr, sizeof(tmpstr), "8259 irq=%s.v3.isa.2 addr=0x%llx",
	    devinit->interrupt_path, (long long)(isa_port_base + 0xa0));
	d->pic2 = devinit->machine->isa_pic_data.pic2 = (struct pic8259_data *)
	    device_add(devinit->machine, tmpstr);

	snprintf(isa_irq_base, sizeof(isa_irq_base), "%s.v3",
	    devinit->interrupt_path);
	snprintf(pci_irq_base, sizeof(pci_irq_base), "%s.v3",
	    devinit->interrupt_path);

	/*  Register a PCI bus:  */
	d->pci_data = bus_pci_init(
	    devinit->machine,
	    pci_irq_base		/*  pciirq: TODO  */,
	    0x1d000000,			/*  pci device io offset  */
	    0x11000000,			/*  pci device mem offset: TODO  */
	    0x00000000,			/*  PCI portbase: TODO  */
	    0x00000000,			/*  PCI membase: TODO  */
	    pci_irq_base,		/*  PCI irqbase  */
	    isa_port_base,		/*  ISA portbase  */
	    0x10000000,			/*  ISA membase  */
	    isa_irq_base);		/*  ISA irqbase  */

	switch (devinit->machine->machine_type) {
	case MACHINE_ALGOR:
		bus_pci_add(devinit->machine, d->pci_data,
		    devinit->machine->memory, 0, 2, 0, "piix3_isa");
		bus_pci_add(devinit->machine, d->pci_data,
		    devinit->machine->memory, 0, 2, 1, "piix3_ide");
		break;
	default:fatal("!\n! WARNING: v3 for non-implemented machine"
		    " type %i\n!\n", devinit->machine->machine_type);
		exit(1);
	}

	/*  PCI configuration space:  */
	memory_device_register(devinit->machine->memory, "v3_pci",
	    0x1ee00000, 0x100000, dev_v3_pci_access, d, DM_DEFAULT, NULL);

	/*  PCI controller:  */
	memory_device_register(devinit->machine->memory, "v3",
	    0x1ef00000, 0x1000, dev_v3_access, d, DM_DEFAULT, NULL);

	devinit->return_ptr = d->pci_data;

	return 1;
}

