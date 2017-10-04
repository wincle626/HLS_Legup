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
 *  COMMENT: USB Open Host Controller Interface
 *
 *  TODO
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

#include "thirdparty/ohcireg.h"


/*  Length is 0x1000 at least on Playstation 2  */
#define	DEV_OHCI_LENGTH		0x1000


#define debug fatal


struct ohci_data {
	struct interrupt	irq;

	int			port1reset;
};


DEVICE_ACCESS(ohci)
{
	struct ohci_data *d = (struct ohci_data *) extra;
	uint64_t idata = 0, odata = 0;
	const char *name = NULL;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	switch (relative_addr) {
	case OHCI_REVISION:
		name = "REVISION";
		if (writeflag == MEM_READ) {
			odata = 0x10;	/*  Version 1.0.  */
		}
		break;
	case OHCI_COMMAND_STATUS:
		name = "COMMAND_STATUS";
		if (idata == 0x2) {
			fatal("Hm... OHCI COMMAND STATUS\n");
			INTERRUPT_ASSERT(d->irq);
		}
		break;
	case OHCI_INTERRUPT_STATUS:
		name = "INTERRUPT_STATUS";
		odata = OHCI_WDH;
		break;
/*
 *  TODO: It now sleeps at      tsleep(xfer, PRIBIO, "usbsyn", 0);
 *  in netbsd/src/sys/dev/usb/usbdi.c
 */
	case OHCI_RH_DESCRIPTOR_A:
		name = "RH_DESCRIPTOR_A";
		odata = 2;	/*  Nr of ports  */
		break;
	case OHCI_RH_STATUS:
		name = "RH_STATUS";
		/*  TODO  */
		break;
	case OHCI_RH_PORT_STATUS(1):	/*  First port  */
		name = "RH_PORT_STATUS(1)";
		if (writeflag == MEM_READ) {
			/*  Status = low 16, Change = top 16  */
			odata = 0x10101;
			/*  0x0001 = connected
			    0x0100 = power  */
			if (d->port1reset)
				odata |= (0x10 << 16) | 0x10;
		} else {
			/*  0x10 = UPS_C_PORT_RESET  */
			if (idata & 0x10)
				d->port1reset = 1;
			if (idata & 0x100000)
				d->port1reset = 0;
		}
		break;
	case OHCI_RH_PORT_STATUS(2):	/*  Second port  */
		name = "RH_PORT_STATUS(2)";
		/*  TODO  */
		odata = 0;
		break;
	default:
		if (writeflag == MEM_READ) {
			debug("[ ohci: read from addr 0x%x: 0x%llx ]\n",
			    (int)relative_addr, (long long)odata);
		} else {
			debug("[ ohci: write to addr 0x%x: 0x%llx ]\n",
			    (int)relative_addr, (long long)idata);
		}
	}

	if (name != NULL) {
		if (writeflag == MEM_READ)
			debug("[ ohci: read from %s: 0x%llx ]\n",
			    name, (long long)odata);
		else
			debug("[ ohci: write to %s: 0x%llx ]\n",
			    name, (long long)idata);
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVINIT(ohci)
{
	struct ohci_data *d;

	CHECK_ALLOCATION(d = (struct ohci_data *) malloc(sizeof(struct ohci_data)));
	memset(d, 0, sizeof(struct ohci_data));

	INTERRUPT_CONNECT(devinit->interrupt_path, d->irq);

	memory_device_register(devinit->machine->memory,
	    devinit->name, devinit->addr,
	    DEV_OHCI_LENGTH, dev_ohci_access, d, DM_DEFAULT, NULL);

	return 1;
}

