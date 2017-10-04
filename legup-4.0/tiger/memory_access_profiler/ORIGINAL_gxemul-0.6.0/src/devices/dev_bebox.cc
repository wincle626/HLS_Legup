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
 *  COMMENT: BeBox motherboard registers and interrupt controller
 *
 *  See the following URL for more information:
 *
 *	http://www.bebox.nu/history.php?s=history/benews/benews27
 *
 *  These interrupt numbers are from NetBSD's bebox/extint.c:
 *
 *	serial3			6
 *	serial4			7
 *	midi1			8
 *	midi2			9
 *	scsi			10
 *	pci1			11
 *	pci2			12
 *	pci3			13
 *	sound			14
 *	8259			26
 *	irda			27
 *	a2d			28
 *	geekport		29
 *
 *  Note that these are in IBM order, i.e. reversed. So 8259 interrupts
 *  go to interrupt 31 - 26 = 5, when using normal numbers.
 *
 *  Interrupt routing should work to both CPUs, but I've only ever seen the
 *  first CPU being used by NetBSD/bebox, so the second CPU is untested :-)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "device.h"
#include "interrupt.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"


struct bebox_data {
	struct interrupt	cpu_irq[2];

	/*  The 5 motherboard registers:  */
	uint32_t		cpu0_int_mask;
	uint32_t		cpu1_int_mask;
	uint32_t		int_status;
	uint32_t		xpi;
	uint32_t		resets;
};


static void bebox_interrupt_assert(struct interrupt *interrupt)
{
	struct bebox_data *d = (struct bebox_data *) interrupt->extra;
	d->int_status |= interrupt->line;

	/*  printf("STATUS %08x  CPU0 %08x  CPU1 %08x\n",
	    d->int_status, d->cpu0_int_mask, d->cpu1_int_mask);  */

	if (d->int_status & d->cpu0_int_mask)
		INTERRUPT_ASSERT(d->cpu_irq[0]);
	if (d->int_status & d->cpu1_int_mask)
		INTERRUPT_ASSERT(d->cpu_irq[1]);
}
static void bebox_interrupt_deassert(struct interrupt *interrupt)
{
	struct bebox_data *d = (struct bebox_data *) interrupt->extra;
	d->int_status &= ~interrupt->line;

	if (!(d->int_status & d->cpu0_int_mask))
		INTERRUPT_DEASSERT(d->cpu_irq[0]);
	if (!(d->int_status & d->cpu1_int_mask))
		INTERRUPT_DEASSERT(d->cpu_irq[1]);
}


/*
 *  check_cpu_masks():
 *
 *  BeBox interrupt enable bits are not allowed to be present in
 *  both CPUs at the same time.
 */
static void check_cpu_masks(struct cpu *cpu, struct bebox_data *d)
{
	d->cpu0_int_mask &= 0x7fffffff;
	d->cpu1_int_mask &= 0x7fffffff;
	if ((d->cpu0_int_mask | d->cpu1_int_mask) !=
	    (d->cpu0_int_mask ^ d->cpu1_int_mask))
		fatal("check_cpu_masks(): BeBox cpu int masks collide!\n");
}


DEVICE_ACCESS(bebox)
{
	struct bebox_data *d = (struct bebox_data *) extra;
	uint64_t idata = 0, odata = 0;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	switch (relative_addr) {

	case 0x0f0:
		if (writeflag == MEM_READ)
			odata = d->cpu0_int_mask;
		else {
			if (idata & 0x80000000)
				d->cpu0_int_mask |= idata;
			else
				d->cpu0_int_mask &= ~idata;
			check_cpu_masks(cpu, d);
		}
		break;

	case 0x1f0:
		if (writeflag == MEM_READ)
			odata = d->cpu1_int_mask;
		else {
			if (idata & 0x80000000)
				d->cpu1_int_mask |= idata;
			else
				d->cpu1_int_mask &= ~idata;
			check_cpu_masks(cpu, d);
		}
		break;

	case 0x2f0:
		if (writeflag == MEM_READ)
			odata = d->int_status;
		else {
			if (idata & 0x80000000)
				d->int_status |= idata;
			else
				d->int_status &= ~idata;
			d->int_status &= 0x7fffffff;
		}
		break;

	case 0x3f0:
		if (writeflag == MEM_READ) {
			odata = d->xpi;

			/*  Bit 6 (counted from the left) is cpuid:  */
			odata &= ~0x02000000;
			if (cpu->cpu_id == 1)
				odata |= 0x02000000;
		} else {
			fatal("[ bebox: unimplemented write to 0x3f0:"
			    " 0x%08x ]\n", (int)idata);
		}
		break;
	default:
		if (writeflag==MEM_READ) {
			fatal("[ bebox: unimplemented read from 0x%08lx ]\n",
			    (long)relative_addr);
		} else {
			fatal("[ bebox: unimplemented write to 0x%08lx: 0x"
			    "%08x ]\n", (long)relative_addr, (int)idata);
		}
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVINIT(bebox)
{
	struct bebox_data *d;
	int i;
	char n[300];
	struct machine *machine = devinit->machine;

	CHECK_ALLOCATION(d = (struct bebox_data *) malloc(sizeof(struct bebox_data)));
	memset(d, 0, sizeof(struct bebox_data));

	/*  Connect to the two BeBox CPUs:  */
	for (i=0; i<2; i++) {
		if (i >= machine->ncpus) {
			fatal("FATAL ERROR: The machine seem to be "
			    "lacking cpu nr %i (0-based)\n", i);
			exit(1);
		}

		snprintf(n, sizeof(n), "%s.cpu[%i]", machine->path, i);
		INTERRUPT_CONNECT(n, d->cpu_irq[i]);
	}

	/*
	 *  Register the 32 BeBox interrupts:
	 *
 	 *  NOTE: They are registered on cpu[0], but the interrupt assert/
	 *  deassert routines in this file make sure that the interrupts
	 *  are routed to the correct cpu!
	 */
	for (i=0; i<32; i++) {
		struct interrupt templ;
		snprintf(n, sizeof(n), "%s.bebox.%i",
		    devinit->interrupt_path, i);
		memset(&templ, 0, sizeof(templ));
		templ.line = 1 << i;
		templ.name = n;
		templ.extra = d;
		templ.interrupt_assert = bebox_interrupt_assert;
		templ.interrupt_deassert = bebox_interrupt_deassert;
		interrupt_handler_register(&templ);
	}

	memory_device_register(machine->memory, devinit->name,
	    0x7ffff000, 0x500, dev_bebox_access, d, DM_DEFAULT, NULL);

	devinit->return_ptr = d;

	return 1;
}

