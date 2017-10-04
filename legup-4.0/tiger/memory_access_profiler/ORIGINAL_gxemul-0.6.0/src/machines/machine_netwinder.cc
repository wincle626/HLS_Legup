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
 *  COMMENT: NetWinder
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


MACHINE_SETUP(netwinder)
{
	char tmpstr[300];
	struct pci_data *pci_bus;

	machine->machine_name = strdup("NetWinder");

	if (machine->physical_ram_in_mb > 256)
		fprintf(stderr, "WARNING! Real NetWinders cannot"
		    " have more than 256 MB RAM. Continuing anyway.\n");

	/*  CPU at 63.75 MHz, according to NetBSD's netwinder_machdep.c.  */
	machine->emulated_hz = 63750000;

	snprintf(tmpstr, sizeof(tmpstr), "footbridge irq=%s.cpu[%i].irq"
	    " addr=0x42000000", machine->path, machine->bootstrap_cpu);
	pci_bus = (struct pci_data *) device_add(machine, tmpstr);

	if (machine->x11_md.in_use) {
		bus_pci_add(machine, pci_bus, machine->memory,
		    0xc0, 8, 0, "igsfb");
	}

	if (!machine->prom_emulation)
		return;

	arm_setup_initial_translation_table(cpu, 0x4000);
}


MACHINE_DEFAULT_CPU(netwinder)
{
	machine->cpu_name = strdup("SA110");
}


MACHINE_DEFAULT_RAM(netwinder)
{
	machine->physical_ram_in_mb = 16;
}


MACHINE_REGISTER(netwinder)
{
	MR_DEFAULT(netwinder, "NetWinder", ARCH_ARM, MACHINE_NETWINDER);

	machine_entry_add_alias(me, "netwinder");

	me->set_default_ram = machine_default_ram_netwinder;
}

