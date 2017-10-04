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
 *  COMMENT: Algor P5064 misc. stuff
 *
 *  TODO: This is hardcoded for P5064 right now. Generalize it to P40xx etc.
 *
 *  CPU irq 2 = ISA, 3 = PCI, 4 = Local.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "device.h"
#include "devices.h"
#include "interrupt.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"

#include "thirdparty/algor_p5064reg.h"


struct algor_data {
	uint64_t		base_addr;
	struct interrupt	mips_irq_2;
	struct interrupt	mips_irq_3;
	struct interrupt	mips_irq_4;
};


DEVICE_ACCESS(algor)
{
	struct algor_data *d = (struct algor_data *) extra;
	uint64_t idata = 0, odata = 0;
	const char *n = NULL;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	relative_addr += d->base_addr;

	switch (relative_addr) {

	case P5064_LED1 + 0x0:
	case P5064_LED1 + 0x4:
	case P5064_LED1 + 0x8:
	case P5064_LED1 + 0xc:
		break;

	case P5064_LOCINT:
		/*
		 *  TODO: See how ISAINT is implemented.
		 *
		 *  Implemented so far: COM1 only.
		 */
		n = "P5064_LOCINT";
		if (writeflag == MEM_READ) {
			/*  Ugly hack for NetBSD startup.  TODO: fix  */
			static int x = 0;
			if (((++ x) & 0xffff) == 0)
				odata |= LOCINT_RTC;

			if (cpu->machine->isa_pic_data.pic1->irr &
			    ~cpu->machine->isa_pic_data.pic1->ier & 0x10)
				odata |= LOCINT_COM1;
			if (cpu->machine->isa_pic_data.pic1->irr &
			    ~cpu->machine->isa_pic_data.pic1->ier & 0x08)
				odata |= LOCINT_COM2;

			/*  Read => ack:  */
			cpu->machine->isa_pic_data.pic1->irr &= ~0x18;
			INTERRUPT_DEASSERT(d->mips_irq_4);
		} else {
			if (idata & LOCINT_COM1)
				cpu->machine->isa_pic_data.pic1->ier &= ~0x10;
			else
				cpu->machine->isa_pic_data.pic1->ier |= 0x10;
			if (idata & LOCINT_COM2)
				cpu->machine->isa_pic_data.pic1->ier &= ~0x08;
			else
				cpu->machine->isa_pic_data.pic1->ier |= 0x08;
		}
		break;

	case P5064_PANIC:
		n = "P5064_PANIC";
		if (writeflag == MEM_READ)
			odata = 0;
		break;

	case P5064_PCIINT:
		/*
		 *  TODO: See how ISAINT is implemented.
		 */
		n = "P5064_PCIINT";
		if (writeflag == MEM_READ) {
			odata = 0;
			INTERRUPT_DEASSERT(d->mips_irq_3);
		}
		break;

	case P5064_ISAINT:
		/*
		 *  ISA interrupts:
		 *
		 *	Bit:  IRQ Source:
		 *	0     ISAINT_ISABR
		 *	1     ISAINT_IDE0
		 *	2     ISAINT_IDE1
		 *
		 *  NOTE/TODO: Ugly redirection to the ISA controller.
		 */
		n = "P5064_ISAINT";
		if (writeflag == MEM_WRITE) {
			if (idata & ISAINT_IDE0)
				cpu->machine->isa_pic_data.pic2->ier &= ~0x40;
			else
				cpu->machine->isa_pic_data.pic2->ier |= 0x40;
			if (idata & ISAINT_IDE1)
				cpu->machine->isa_pic_data.pic2->ier &= ~0x80;
			else
				cpu->machine->isa_pic_data.pic2->ier |= 0x80;
			cpu->machine->isa_pic_data.pic1->ier &= ~0x04;
		} else {
			if (cpu->machine->isa_pic_data.pic2->irr &
			    ~cpu->machine->isa_pic_data.pic2->ier & 0x40)
				odata |= ISAINT_IDE0;
			if (cpu->machine->isa_pic_data.pic2->irr &
			    ~cpu->machine->isa_pic_data.pic2->ier & 0x80)
				odata |= ISAINT_IDE1;

			/*  Read => ack:  */
			cpu->machine->isa_pic_data.pic2->irr &= ~0xc0;
			INTERRUPT_DEASSERT(d->mips_irq_2);
		}
		break;

	case P5064_KBDINT:
		/*
		 *  TODO: See how ISAINT is implemented.
		 */
		n = "P5064_KBDINT";
		if (writeflag == MEM_READ)
			odata = 0;
		break;

	default:if (writeflag == MEM_READ) {
			fatal("[ algor: read from 0x%x ]\n",
			    (int)relative_addr);
		} else {
			fatal("[ algor: write to 0x%x: 0x%"PRIx64" ]\n",
			    (int) relative_addr, (uint64_t) idata);
		}
	}

	if (n != NULL) {
		if (writeflag == MEM_READ) {
			debug("[ algor: read from %s: 0x%"PRIx64" ]\n",
			    n, (uint64_t) odata);
		} else {
			debug("[ algor: write to %s: 0x%"PRIx64" ]\n",
			    n, (uint64_t) idata);
		}
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVINIT(algor)
{
	char tmpstr[200];
	struct algor_data *d;

	CHECK_ALLOCATION(d = (struct algor_data *) malloc(sizeof(struct algor_data)));
	memset(d, 0, sizeof(struct algor_data));

	d->base_addr = devinit->addr;
	if (devinit->addr != 0x1ff00000) {
		fatal("The Algor base address should be 0x1ff00000.\n");
		exit(1);
	}

	/*  Connect to MIPS irq 2, 3, and 4:  */
	snprintf(tmpstr, sizeof(tmpstr), "%s.2", devinit->interrupt_path);
	INTERRUPT_CONNECT(tmpstr, d->mips_irq_2);
	snprintf(tmpstr, sizeof(tmpstr), "%s.3", devinit->interrupt_path);
	INTERRUPT_CONNECT(tmpstr, d->mips_irq_3);
	snprintf(tmpstr, sizeof(tmpstr), "%s.4", devinit->interrupt_path);
	INTERRUPT_CONNECT(tmpstr, d->mips_irq_4);

	memory_device_register(devinit->machine->memory, devinit->name,
	    devinit->addr, 0x100000, dev_algor_access, d, DM_DEFAULT, NULL);

	devinit->return_ptr = d;

	return 1;
}

