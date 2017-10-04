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
 *  COMMENT: Artesyn's PM/PPC board
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bus_pci.h"
#include "cpu.h"
#include "device.h"
#include "devices.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"



MACHINE_SETUP(pmppc)
{
	struct pci_data *pci_data;
	char tmpstr[300];

	/*
	 *  NetBSD/pmppc (http://www.netbsd.org/ports/pmppc/)
	 */
	machine->machine_name = strdup("Artesyn's PM/PPC board");

	/*  Bogus default speed = 33 MHz  */
	if (machine->emulated_hz == 0)
		machine->emulated_hz = 33000000;

	/*  PM/PPC specific motherboard registers:  */
	device_add(machine, "pmppc");

	/*  PCI and Interrupt controller:  */
	pci_data = (struct pci_data *) device_add(machine, "cpc700");

	/*  RTC at "ext int 5" = "int 25" in IBM jargon, int
	    31-25 = 6 for the rest of us.  */
	snprintf(tmpstr, sizeof(tmpstr), "%s.cpu[%i].cpc700.%i",
	    machine->path, machine->bootstrap_cpu, 31-25);
	dev_mc146818_init(machine, machine->memory, 0x7ff00000, tmpstr,
	    MC146818_PMPPC, 1);

	bus_pci_add(machine, pci_data, machine->memory, 0, 8, 0, "dec21143");
}


MACHINE_DEFAULT_CPU(pmppc)
{
	machine->cpu_name = strdup("PPC750");
}


MACHINE_REGISTER(pmppc)
{
	MR_DEFAULT(pmppc, "Artesyn's PM/PPC board", ARCH_PPC, MACHINE_PMPPC);

	machine_entry_add_alias(me, "pmppc");
}

