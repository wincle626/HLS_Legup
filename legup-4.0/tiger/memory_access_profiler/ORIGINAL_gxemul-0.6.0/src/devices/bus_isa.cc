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
 *  COMMENT: Generic ISA bus framework
 *
 *  This is not a normal device, but it can be used as a quick way of adding
 *  most of the common legacy ISA devices to a machine.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUS_ISA_C

#include "bus_isa.h"
#include "device.h"
#include "devices.h"
#include "diskimage.h"
#include "interrupt.h"
#include "machine.h"
#include "misc.h"


/*
 *  isa_interrupt_common():
 */
void isa_interrupt_common(struct bus_isa_data *d, int old_isa_assert)
{
	int new_isa_assert, x;

	/*  Any interrupt assertions on PIC2 go to irq 2 on PIC1  */
	/*  (TODO: don't hardcode this here)  */
	if (d->pic2->irr & ~d->pic2->ier)
		d->pic1->irr |= 0x04;
	else
		d->pic1->irr &= ~0x04;

	/*  printf("ISA: irr=%02x%02x ier=%02x%02x\n",
	    d->pic2->irr, d->pic1->irr, d->pic2->ier, d->pic1->ier);  */

	new_isa_assert = d->pic1->irr & ~d->pic1->ier;

	if (old_isa_assert == new_isa_assert)
		return;

	if (!new_isa_assert) {
		INTERRUPT_DEASSERT(d->irq);
		return;
	}

	for (x=0; x<16; x++) {
		if (x == 2)
			continue;

		if (x < 8 && (d->pic1->irr & ~d->pic1->ier & (1 << x)))
			break;

		if (x >= 8 && (d->pic2->irr & ~d->pic2->ier & (1 << (x&7))))
			break;
	}

	*d->ptr_to_last_int = x;

	INTERRUPT_ASSERT(d->irq);
}


/*
 *  isa_interrupt_assert():
 *
 *  Called whenever an ISA device asserts an interrupt (0..15).
 */
void isa_interrupt_assert(struct interrupt *interrupt)
{
	struct bus_isa_data *d = (struct bus_isa_data *) interrupt->extra;
	int old_isa_assert, line = interrupt->line;
	int mask = 1 << (line & 7);

	old_isa_assert = d->pic1->irr & ~d->pic1->ier;

	if (line < 8)
		d->pic1->irr |= mask;
	else if (d->pic2 != NULL)
		d->pic2->irr |= mask;

	isa_interrupt_common(d, old_isa_assert);
}


/*
 *  isa_interrupt_deassert():
 *
 *  Called whenever an ISA device deasserts an interrupt (0..15).
 */
void isa_interrupt_deassert(struct interrupt *interrupt)
{
	struct bus_isa_data *d = (struct bus_isa_data *) interrupt->extra;
	int line = interrupt->line, mask = 1 << (line & 7);
	int old_irr1 = d->pic1->irr, old_isa_assert;

	old_isa_assert = old_irr1 & ~d->pic1->ier;

	if (line < 8)
		d->pic1->irr &= ~mask;
	else if (d->pic2 != NULL)
		d->pic2->irr &= ~mask;

	/*  If IRQ 0 has been cleared, then this is a timer interrupt.
	    Let's ack it here:  */
	if (old_irr1 & 1 && !(d->pic1->irr & 1) &&
	    d->ptr_to_pending_timer_interrupts != NULL &&
            (*d->ptr_to_pending_timer_interrupts) > 0)
                (*d->ptr_to_pending_timer_interrupts) --;

	isa_interrupt_common(d, old_isa_assert);
}


/*
 *  bus_isa_init():
 *
 *  Flags are zero or more of the following, ORed together:
 *
 *  BUS_ISA_EXTERNAL_PIC	Don't register/use isa_interrupt_*().
 *  BUS_ISA_IDE0		Include wdc0.
 *  BUS_ISA_IDE1		Include wdc1.
 *  BUS_ISA_FDC			Include a floppy controller. (Dummy.)
 *  BUS_ISA_VGA			Include old-style (non-PCI) VGA. (*1)
 *  BUS_ISA_VGA_FORCE		Include VGA even when running without X11. (*2)
 *  BUS_ISA_PCKBC_FORCE_USE	Always assume keyboard console, not serial. (*3)
 *  BUS_ISA_PCKBC_NONPCSTYLE	Don't set the pc-style flag for the keyboard.
 *  BUS_ISA_NO_SECOND_PIC	Only useful for 8086 XT (pre-AT) emulation. :-)
 *  BUS_ISA_LPTBASE_3BC		Set lptbase to 0x3bc instead of 0x378.
 *
 *  (*1) For machines with a PCI bus, this flag should not be used. Instead, a
 *       PCI VGA card should be added to the PCI bus.
 *
 *  (*2) For machines where it is easy to select VGA vs serial console during
 *       boot, this flag should not be used. Machines that "always" boot up
 *       in VGA console mode should have it set.
 *
 *  (*3) Similar to *2 above; machines that always boot up with VGA console
 *       should have this flag set, so that the keyboard is always used.
 *
 *  The interrupt_base_path is the name of the bus, CPU, or controller onto
 *  which this ISA bus will be attached, e.g. "machine[0].lca" or
 *  "machine[0].cpu[0].pic1".
 */
struct bus_isa_data *bus_isa_init(struct machine *machine,
	char *interrupt_base_path, uint32_t bus_isa_flags,
	uint64_t isa_portbase, uint64_t isa_membase)
{
	struct bus_isa_data *d;
	char tmpstr[300], tmpstr2[300];
	int wdc0_irq = 14, wdc1_irq = 15;
	int i, tmp_handle, kbd_in_use;
	int lptbase = 0x378;

	CHECK_ALLOCATION(d = (struct bus_isa_data *) malloc(sizeof(struct bus_isa_data)));
	memset(d, 0, sizeof(struct bus_isa_data));

	d->isa_portbase = isa_portbase;
	d->isa_membase  = isa_membase;

	if (!(bus_isa_flags & BUS_ISA_EXTERNAL_PIC)) {
		/*  Connect to the interrupt which we're interrupting
		    at (usually a CPU):  */
		INTERRUPT_CONNECT(interrupt_base_path, d->irq);

		/*  Register the 16 possible ISA interrupts:  */
		for (i=0; i<16; i++) {
			struct interrupt templ;
			char name[300];
			snprintf(name, sizeof(name),
			    "%s.isa.%i", interrupt_base_path, i);
			memset(&templ, 0, sizeof(templ));
			templ.line = i;
			templ.name = name;
			templ.extra = d;
			templ.interrupt_assert = isa_interrupt_assert;
			templ.interrupt_deassert = isa_interrupt_deassert;
			interrupt_handler_register(&templ);
		}
	}

	kbd_in_use = ((bus_isa_flags & BUS_ISA_PCKBC_FORCE_USE) ||
	    (machine->x11_md.in_use))? 1 : 0;

	if (machine->machine_type == MACHINE_PREP) {
		/*  PReP with obio controller has both WDCs on irq 13!  */
		wdc0_irq = wdc1_irq = 13;
	}

	if (!(bus_isa_flags & BUS_ISA_EXTERNAL_PIC)) {
		snprintf(tmpstr, sizeof(tmpstr), "8259 irq=%s addr=0x%llx",
		    interrupt_base_path, (long long)(isa_portbase + 0x20));
		d->pic1 = machine->isa_pic_data.pic1 = (struct pic8259_data *)
		    device_add(machine, tmpstr);
		d->ptr_to_pending_timer_interrupts =
		    machine->isa_pic_data.pending_timer_interrupts;
		d->ptr_to_last_int = &machine->isa_pic_data.last_int;

		if (bus_isa_flags & BUS_ISA_NO_SECOND_PIC)
			bus_isa_flags &= ~BUS_ISA_NO_SECOND_PIC;
		else {
			snprintf(tmpstr, sizeof(tmpstr),
			    "8259 irq=%s.isa.2 addr=0x%llx",
			    interrupt_base_path,(long long)(isa_portbase+0xa0));
			d->pic2 = machine->isa_pic_data.pic2 = (struct pic8259_data *)
			    device_add(machine, tmpstr);
		}
	} else {
		bus_isa_flags &= ~BUS_ISA_EXTERNAL_PIC;
	}

	snprintf(tmpstr, sizeof(tmpstr), "8253 irq=%s.isa.%i addr=0x%llx "
	    "in_use=0", interrupt_base_path, 0,
	    (long long)(isa_portbase + 0x40));
	device_add(machine, tmpstr);

	snprintf(tmpstr, sizeof(tmpstr), "pccmos irq=%s.isa.%i addr=0x%llx",
	    interrupt_base_path, 8, (long long)(isa_portbase + 0x70));
	device_add(machine, tmpstr);

	snprintf(tmpstr, sizeof(tmpstr), "ns16550 irq=%s.isa.%i addr=0x%llx "
	    "name2=tty0 in_use=%i", interrupt_base_path, 4,
	    (long long)(isa_portbase + 0x3f8), 1 - kbd_in_use);
	machine->main_console_handle = (size_t)device_add(machine, tmpstr);

	snprintf(tmpstr, sizeof(tmpstr), "ns16550 irq=%s.isa.%i addr=0x%llx "
	    "name2=tty1 in_use=0", interrupt_base_path, 3,
	    (long long)(isa_portbase + 0x2f8));
	device_add(machine, tmpstr);

	if (bus_isa_flags & BUS_ISA_LPTBASE_3BC) {
		bus_isa_flags &= ~BUS_ISA_LPTBASE_3BC;
		lptbase = 0x3bc;
	}

	snprintf(tmpstr, sizeof(tmpstr), "lpt irq=%s.isa.%i addr=0x%llx "
	    "name2=lpt in_use=0", interrupt_base_path, 7,
	    (long long)(isa_portbase + lptbase));
	device_add(machine, tmpstr);

	if (bus_isa_flags & BUS_ISA_IDE0) {
		bus_isa_flags &= ~BUS_ISA_IDE0;
		snprintf(tmpstr, sizeof(tmpstr), "wdc irq=%s.isa.%i "
		    "addr=0x%llx", interrupt_base_path, wdc0_irq,
		    (long long)(isa_portbase + 0x1f0));
		if (diskimage_exist(machine, 0, DISKIMAGE_IDE) ||
		    diskimage_exist(machine, 1, DISKIMAGE_IDE))
			device_add(machine, tmpstr);
	}

	if (bus_isa_flags & BUS_ISA_IDE1) {
		bus_isa_flags &= ~BUS_ISA_IDE1;
		snprintf(tmpstr, sizeof(tmpstr), "wdc irq=%s.isa.%i "
		    "addr=0x%llx", interrupt_base_path, wdc1_irq,
		    (long long)(isa_portbase + 0x170));
		if (diskimage_exist(machine, 2, DISKIMAGE_IDE) ||
		    diskimage_exist(machine, 3, DISKIMAGE_IDE))
			device_add(machine, tmpstr);
	}

	if (bus_isa_flags & BUS_ISA_FDC) {
		bus_isa_flags &= ~BUS_ISA_FDC;
		snprintf(tmpstr, sizeof(tmpstr), "fdc irq=%s.isa.%i "
		    "addr=0x%llx", interrupt_base_path, 6,
		    (long long)(isa_portbase + 0x3f0));
		device_add(machine, tmpstr);
	}

	if (bus_isa_flags & BUS_ISA_VGA) {
		if (machine->x11_md.in_use || bus_isa_flags & BUS_ISA_VGA_FORCE)
			dev_vga_init(machine, machine->memory,
			    isa_membase + 0xa0000, isa_portbase + 0x3c0,
			    machine->machine_name);
		bus_isa_flags &= ~(BUS_ISA_VGA | BUS_ISA_VGA_FORCE);
	}

	snprintf(tmpstr, sizeof(tmpstr), "%s.isa.1", interrupt_base_path);
	snprintf(tmpstr2, sizeof(tmpstr2), "%s.isa.12", interrupt_base_path);
	tmp_handle = dev_pckbc_init(machine, machine->memory,
	    isa_portbase + 0x60, PCKBC_8042, tmpstr, tmpstr2,
	    kbd_in_use, bus_isa_flags & BUS_ISA_PCKBC_NONPCSTYLE? 0 : 1);

	if (kbd_in_use)
		machine->main_console_handle = tmp_handle;

	bus_isa_flags &= ~(BUS_ISA_PCKBC_NONPCSTYLE | BUS_ISA_PCKBC_FORCE_USE);

	if (bus_isa_flags != 0)
		fatal("WARNING! bus_isa(): unimplemented bus_isa_flags 0x%x\n",
		    bus_isa_flags);

	return d;
}

