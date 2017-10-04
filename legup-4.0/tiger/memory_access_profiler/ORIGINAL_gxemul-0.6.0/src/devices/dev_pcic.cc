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
 *  COMMENT: Intel 82365SL PC Card Interface Controller
 *
 *  (Called "pcic" by NetBSD.)
 *
 *  TODO: Lots of stuff. This is just a quick hack. Don't rely on it.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "device.h"
#include "emul.h"
#include "interrupt.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"

#include "thirdparty/i82365reg.h"
#include "thirdparty/pcmciareg.h"


/*  #define debug fatal  */

#define	DEV_PCIC_LENGTH		2

struct pcic_data {
	struct interrupt	irq;
	int			regnr;
};


DEVICE_ACCESS(pcic_cis)
{
	/*  struct pcic_data *d = (struct pcic_data *) extra;  */
	uint64_t idata = 0, odata = 0;

	idata = memory_readmax64(cpu, data, len);

{
#if 0
	/*  SMC, PCM Ethernet Adapter, CIS V1.05 (manufacturer 0x108, 
	    product 0x105)  */
	unsigned char x[] = {
		PCMCIA_CISTPL_DEVICE, 3, PCMCIA_DTYPE_FUNCSPEC, 0xff,0xff,
		PCMCIA_CISTPL_FUNCID, 2, 0x06, 0x00,
		PCMCIA_CISTPL_MANFID, 4, 0x08, 0x01, 0x05, 0x01,
		PCMCIA_CISTPL_VERS_1, 0x26,
		0x04, 0x01, 0x53, 0x4d, 0x43, 0x00, 0x50, 0x43, 0x4d, 0x20,
		0x45, 0x74, 0x68, 0x65, 0x72, 0x6e, 0x65, 0x74, 0x20, 0x41,
		0x64, 0x61, 0x70, 0x74, 0x65, 0x72, 0x00, 0x43, 0x49, 0x53,
		0x20, 0x56, 0x31, 0x2e, 0x30, 0x35, 0x00, 0xff,
		PCMCIA_CISTPL_CONFIG, 0x0a,
		0x02, 0x01, 0x00, 0x00, 0x01, 0x03, 0x00, 0x00, 0x00, 0xff,
		PCMCIA_CISTPL_CFTABLE_ENTRY, 0x0b,
		0xc1, 0x01, 0x70, 0x50, 0xbc, 0x8e, 0x48, 0x40, 0x00,0x02,0xff,
		/*  unhandled CISTPL 22  */
		0x22, 0x02, 0x01, 0x02,
		/*  unhandled CISTPL 22  */
		0x22, 0x05, 0x02, 0x80, 0x96, 0x98, 0x00,
		/*  unhandled CISTPL 22  */
		0x22, 0x02, 0x03, 0x01,
		/*  unhandled CISTPL 22  */
		0x22, 0x08, 0x04, 0x06, 0x00, 0x00, 0xc0, 0x2f, 0x48, 0xd2,
		/*  unhandled CISTPL 22  */
		0x22, 0x02, 0x05, 0x01,

		PCMCIA_CISTPL_END, 0
	};
#endif

	/*  From http://www.mail-archive.com/freebsd-current@freebsd.
		org/msg32550.html  */
	unsigned char x[] = {
		PCMCIA_CISTPL_DEVICE, 3, 0xdc, 0x00, 0xff,
		PCMCIA_CISTPL_VERS_1, 0x1a,
		0x04,0x01,0x20,0x00,0x4e,0x69,0x6e,0x6a,0x61,0x41,0x54,0x41,
		0x2d,0x00,0x56,0x31,0x2e,0x30,0x00,0x41,0x50,0x30,0x30,0x20,
		0x00,0xff,
		PCMCIA_CISTPL_CONFIG, 5,
		0x01,0x23,0x00,0x02,0x03,
		PCMCIA_CISTPL_CFTABLE_ENTRY, 0x15,
		0xe1,0x01,0x3d,0x11,0x55,0x1e,0xfc,0x23,0xf0,0x61,0x80,0x01,
		0x07,0x86,0x03,0x01,0x30,0x68,0xd0,0x10,0x00,
#if 0
		PCMCIA_CISTPL_CFTABLE_ENTRY, 0xf,
		0x22,0x38,0xf0,0x61,0x90,0x01,0x07,0x96,0x03,0x01,0x30,0x68,
		0xd0,0x10,0x00,
		PCMCIA_CISTPL_CFTABLE_ENTRY, 0xf,
		0x23,0x38,0xf0,0x61,0xa0,0x01,0x07,0xa6,0x03,0x01,0x30,0x68,
		0xd0,0x10,0x00,
#endif
		PCMCIA_CISTPL_NO_LINK, 0,

		PCMCIA_CISTPL_END, 0
	};

	relative_addr /= 2;
	if (relative_addr < sizeof(x))
		odata = x[relative_addr];

	debug("[ dev_pcic_cis_access: blah blah: addr=0x%x ]\n",
	    (int)relative_addr);
}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVICE_ACCESS(pcic)
{
	struct pcic_data *d = (struct pcic_data *) extra;
	uint64_t idata = 0, odata = 0;
	int controller_nr, socket_nr;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	controller_nr = d->regnr & 0x80? 1 : 0;
	socket_nr = d->regnr & 0x40? 1 : 0;

	switch (relative_addr) {

	case 0:	/*  Register select:  */
		if (writeflag == MEM_WRITE)
			d->regnr = idata;
		else
			odata = d->regnr;
		break;

	case 1:	/*  Register access:  */
		switch (d->regnr & 0x3f) {

		case PCIC_IDENT:
			/*  This causes sockets A and B to be present on
			    controller 0, and only socket A on controller 1.  */
			if (controller_nr == 1 && socket_nr == 1)
				odata = 0;
			else
				odata = PCIC_IDENT_IFTYPE_MEM_AND_IO
				    | PCIC_IDENT_REV_I82365SLR1;
			break;
#if 1
		case PCIC_INTR:
			odata = PCIC_INTR_IRQ3;
			break;
#endif

		case PCIC_CSC:
			odata = PCIC_CSC_GPI;
			break;

		case PCIC_IF_STATUS:
			odata = PCIC_IF_STATUS_READY
			    | PCIC_IF_STATUS_POWERACTIVE;
			if (controller_nr == 0 && socket_nr == 0)
				odata |= PCIC_IF_STATUS_CARDDETECT_PRESENT;
			break;

		default:
			if (writeflag == MEM_WRITE) {
				debug("[ pcic: unimplemented write to "
				    "controller %i socket %c, regnr %i: "
				    "data=0x%02x ]\n", controller_nr,
				    socket_nr? 'B' : 'A',
				    d->regnr & 0x3f, (int)idata);
			} else {
				debug("[ pcic: unimplemented read from "
				    "controller %i socket %c, regnr %i ]\n",
				    controller_nr, socket_nr? 'B' : 'A',
				    d->regnr & 0x3f);
			}
		}
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVINIT(pcic)
{
	char tmpstr[200];
	struct pcic_data *d;

	CHECK_ALLOCATION(d = (struct pcic_data *) malloc(sizeof(struct pcic_data)));
	memset(d, 0, sizeof(struct pcic_data));

	INTERRUPT_CONNECT(devinit->interrupt_path, d->irq);

	memory_device_register(devinit->machine->memory, devinit->name,
	    devinit->addr, DEV_PCIC_LENGTH,
	    dev_pcic_access, (void *)d, DM_DEFAULT, NULL);

	/*  TODO: this shouldn't be hardcoded for hpcmips here!  */
	memory_device_register(devinit->machine->memory, "pcic_cis",
	    0x10070000, 0x1000, dev_pcic_cis_access, (void *)d,
	    DM_DEFAULT, NULL);

	/*  TODO: find out a good way to specify the address, and the IRQ!  */
	snprintf(tmpstr, sizeof(tmpstr), "wdc addr=0x14000180 irq=%s.giu.9",
	    devinit->interrupt_path);
	device_add(devinit->machine, tmpstr);

	/*  TODO: Linux/MobilePro looks at 0x14000170 and 0x1f0...  */
	/*  Yuck. Now there are two. How should this be solved nicely?  */
	snprintf(tmpstr, sizeof(tmpstr), "wdc addr=0x140001f0 irq=%s.giu.9",
	    devinit->interrupt_path);
	device_add(devinit->machine, tmpstr);

	return 1;
}

