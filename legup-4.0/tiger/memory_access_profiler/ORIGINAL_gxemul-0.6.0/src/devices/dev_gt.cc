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
 *  COMMENT: Galileo Technology GT-64xxx PCI controller
 *
 *	GT-64011	Used in Cobalt machines.
 *	GT-64120	Used in evbmips machines (Malta).
 *	GT-64260	Used in mvmeppc machines.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bus_pci.h"
#include "cpu.h"
#include "devices.h"
#include "interrupt.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"
#include "timer.h"

#include "thirdparty/gtreg.h"


#define	TICK_SHIFT		14

/*  #define debug fatal  */

#define PCI_PRODUCT_GALILEO_GT64011  0x4146    /*  GT-64011  */
#define	PCI_PRODUCT_GALILEO_GT64120  0x4620    /*  GT-64120  */
#define	PCI_PRODUCT_GALILEO_GT64260  0x6430    /*  GT-64260  */


struct gt_data {
	int		type;

	struct timer	*timer;
	struct interrupt timer0_irq;
	int		interrupt_hz;
	int		pending_timer0_interrupts;

	/*  Address decode registers:  */
	uint32_t	decode[GT_N_DECODE_REGS];

	struct pci_data	*pci_data;
};


/*
 *  timer_tick():
 *
 *  Called d->interrupt_hz times per (real-world) second.
 */
static void timer_tick(struct timer *timer, void *extra)
{
	struct gt_data *d = (struct gt_data *) extra;
	d->pending_timer0_interrupts ++;
}


DEVICE_TICK(gt)
{
	struct gt_data *d = (struct gt_data *) extra;
	if (d->pending_timer0_interrupts > 0)
		INTERRUPT_ASSERT(d->timer0_irq);
}


DEVICE_ACCESS(gt)
{
	struct gt_data *d = (struct gt_data *) extra;
	uint64_t idata = 0, odata = 0;
	int bus, dev, func, reg;
	size_t i;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	switch (relative_addr) {

	case GT_PCI0IOLD_OFS:
	case GT_PCI0IOHD_OFS:
	case GT_PCI0M0LD_OFS:
	case GT_PCI0M0HD_OFS:
	case GT_PCI0M1LD_OFS:
	case GT_PCI0M1HD_OFS:
	case GT_PCI0IOREMAP_OFS:
	case GT_PCI0M0REMAP_OFS:
	case GT_PCI0M1REMAP_OFS:
		if (writeflag == MEM_READ) {
			odata = d->decode[relative_addr / 8];
			debug("[ gt: read from offset 0x%x: 0x%x ]\n",
			    (int)relative_addr, (int)odata);
		} else {
			d->decode[relative_addr / 8] = idata;
			fatal("[ gt: write to offset 0x%x: 0x%x (TODO) ]\n",
			    (int)relative_addr, (int)idata);
		}
		break;

	case GT_PCI0_CMD_OFS:
		if (writeflag == MEM_WRITE) {
			debug("[ gt: write to GT_PCI0_CMD: 0x%08x (TODO) ]\n",
			    (int)idata);
		} else {
			debug("[ gt: read from GT_PCI0_CMD (0x%08x) (TODO) ]\n",
			    (int)odata);
		}
		break;

	case GT_INTR_CAUSE:
		if (writeflag == MEM_WRITE) {
			debug("[ gt: write to GT_INTR_CAUSE: 0x%08x ]\n",
			    (int)idata);
			return 1;
		} else {
			odata = GTIC_T0EXP;
			INTERRUPT_DEASSERT(d->timer0_irq);

			if (d->pending_timer0_interrupts > 0)
				d->pending_timer0_interrupts --;

			debug("[ gt: read from GT_INTR_CAUSE (0x%08x) ]\n",
			    (int)odata);
		}
		break;

	case GT_PCI0_INTR_ACK:
		odata = cpu->machine->isa_pic_data.last_int;
		/*  TODO: Actually ack the interrupt?  */
		break;

	case GT_TIMER_CTRL:
		if (writeflag == MEM_WRITE) {
			if (idata & ENTC0) {
				/*  TODO: Don't hardcode this.  */
				d->interrupt_hz = 100;
				if (d->timer == NULL)
					d->timer = timer_add(d->interrupt_hz,
					    timer_tick, d);
				else
					timer_update_frequency(d->timer,
					    d->interrupt_hz);
			}
		}
		break;

	case GT_PCI0_CFG_ADDR:
		if (cpu->byte_order != EMUL_LITTLE_ENDIAN) {
			fatal("[ gt: TODO: big endian PCI access ]\n");
			exit(1);
		}
		bus_pci_decompose_1(idata, &bus, &dev, &func, &reg);
		bus_pci_setaddr(cpu, d->pci_data, bus, dev, func, reg);
		break;

	case GT_PCI0_CFG_DATA:
		if (cpu->byte_order != EMUL_LITTLE_ENDIAN) {
			fatal("[ gt: TODO: big endian PCI access ]\n");
			exit(1);
		}
		bus_pci_data_access(cpu, d->pci_data, writeflag == MEM_READ?
		    &odata : &idata, len, writeflag);
		break;

	default:
		if (writeflag == MEM_READ) {
			debug("[ gt: read from addr 0x%x ]\n",
			    (int)relative_addr);
		} else {
			debug("[ gt: write to addr 0x%x:", (int)relative_addr);
			for (i=0; i<len; i++)
				debug(" %02x", data[i]);
			debug(" ]\n");
		}
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


/*
 *  dev_gt_init():
 *
 *  Initialize a Gallileo PCI controller device. First, the controller itself
 *  is added to the bus, then a pointer to the bus is returned.
 */
struct pci_data *dev_gt_init(struct machine *machine, struct memory *mem,
	uint64_t baseaddr, const char *timer_irq_path, const char *isa_irq_path, int type)
{
	struct gt_data *d;
	uint64_t pci_portbase = 0, pci_membase = 0;
	uint64_t isa_portbase = 0, isa_membase = 0;
	uint64_t pci_io_offset = 0, pci_mem_offset = 0;
	const char *gt_name = "NO";

	CHECK_ALLOCATION(d = (struct gt_data *) malloc(sizeof(struct gt_data)));
	memset(d, 0, sizeof(struct gt_data));

	INTERRUPT_CONNECT(timer_irq_path, d->timer0_irq);

	switch (type) {
	case 11:
		/*  Cobalt:  */
		d->type = PCI_PRODUCT_GALILEO_GT64011;
		gt_name = "gt64011";
		pci_io_offset = 0;
		pci_mem_offset = 0;
		pci_portbase = 0x10000000ULL;
		pci_membase = 0x10100000ULL;
		isa_portbase = 0x10000000ULL;
		isa_membase = 0x10100000ULL;
		break;
	case 120:
		/*  EVBMIPS (Malta):  */
		d->type = PCI_PRODUCT_GALILEO_GT64120;
		gt_name = "gt64120";
		pci_io_offset = 0;
		pci_mem_offset = 0;
		pci_portbase = 0x18000000ULL;
		pci_membase = 0x10000000ULL;
		isa_portbase = 0x18000000ULL;
		isa_membase = 0x10000000ULL;
		break;
	case 260:
		/*  MVMEPPC (mvme5500):  */
		d->type = PCI_PRODUCT_GALILEO_GT64260;
		gt_name = "gt64260";
		pci_io_offset = 0;
		pci_mem_offset = 0;
		pci_portbase = 0x18000000ULL;
		pci_membase = 0x10000000ULL;
		isa_portbase = 0x18000000ULL;
		isa_membase = 0x10000000ULL;
		break;
	default:fatal("dev_gt_init(): unimplemented GT type (%i).\n", type);
		exit(1);
	}


	/*
	 *  TODO: FIX THESE! Hardcoded numbers = bad.
	 */
	d->decode[GT_PCI0IOLD_OFS / 8] = pci_portbase >> 21;
	d->decode[GT_PCI0IOHD_OFS / 8] = 0x40;
	d->decode[GT_PCI0M0LD_OFS / 8] = 0x80;
	d->decode[GT_PCI0M0HD_OFS / 8] = 0x3f;
	d->decode[GT_PCI0M1LD_OFS / 8] = 0xc1;
	d->decode[GT_PCI0M1HD_OFS / 8] = 0x5e;
	d->decode[GT_PCI0IOREMAP_OFS / 8] = d->decode[GT_PCI0IOLD_OFS / 8];
	d->decode[GT_PCI0M0REMAP_OFS / 8] = d->decode[GT_PCI0M0LD_OFS / 8];
	d->decode[GT_PCI0M1REMAP_OFS / 8] = d->decode[GT_PCI0M1LD_OFS / 8];

	d->pci_data = bus_pci_init(machine,
	    "TODO_gt_irq", pci_io_offset, pci_mem_offset,
	    pci_portbase, pci_membase, "TODO_pci_irqbase",
	    isa_portbase, isa_membase, isa_irq_path);

	/*
	 *  According to NetBSD/cobalt:
	 *  pchb0 at pci0 dev 0 function 0: Galileo GT-64011
	 *  System Controller, rev 1
	 */
	bus_pci_add(machine, d->pci_data, mem, 0, 0, 0, gt_name);

	memory_device_register(mem, "gt", baseaddr, DEV_GT_LENGTH,
	    dev_gt_access, d, DM_DEFAULT, NULL);
	machine_add_tickfunction(machine, dev_gt_tick, d, TICK_SHIFT);

	return d->pci_data;
}

