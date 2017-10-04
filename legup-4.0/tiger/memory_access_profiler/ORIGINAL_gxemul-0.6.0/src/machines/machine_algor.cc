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
 *  COMMENT: Algorithmic P4032 and P5064 evaluation boards
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bus_isa.h"
#include "bus_pci.h"
#include "cpu.h"
#include "device.h"
#include "devices.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"


MACHINE_SETUP(algor)
{
	struct pci_data *pci_bus;
	char tmpstr[300];

	machine->emulated_hz = 166560000;

	switch (machine->machine_subtype) {
	case MACHINE_ALGOR_P4032:
		machine->machine_name = strdup("Algor P4032");
		break;
	case MACHINE_ALGOR_P5064:
		machine->machine_name = strdup("Algor P5064");
		break;
	default:fatal("Unimplemented Algor machine.\n");
		exit(1);
	}

	/*
	 *  Algor CPU interrupts:
	 *
	 *  7 = CPU count/compare
	 *  4 = Local
	 *  3 = PCI
	 *  2 = ISA
	 */

	pci_bus = (struct pci_data *) device_add(machine, "v3");

	device_add(machine, "algor addr=0x1ff00000");

	snprintf(tmpstr, sizeof(tmpstr), "%s.cpu[%i].v3",
	    machine->path, machine->bootstrap_cpu);
	bus_isa_init(machine, tmpstr, BUS_ISA_EXTERNAL_PIC | BUS_ISA_FDC,
	    0x1d000000, 0x10000000);

	/*  bus_pci_add(machine, pci_bus, machine->memory, 0, 0, 0,
	    "dec21143");  */

	if (!machine->prom_emulation)
		return;

	/*  NetBSD/algor wants these:  */

	/*  a0 = argc  */
	cpu->cd.mips.gpr[MIPS_GPR_A0] = 2;

	/*  a1 = argv  */
	cpu->cd.mips.gpr[MIPS_GPR_A1] = (int32_t)0x9fc01000;
	store_32bit_word(cpu, (int32_t)0x9fc01000, 0x9fc01040);
	store_32bit_word(cpu, (int32_t)0x9fc01004, 0x9fc01200);
	store_32bit_word(cpu, (int32_t)0x9fc01008, 0);

	machine->bootstr = strdup(machine->boot_kernel_filename);
	machine->bootarg = strdup(machine->boot_string_argument);
	store_string(cpu, (int32_t)0x9fc01040, machine->bootstr);
	store_string(cpu, (int32_t)0x9fc01200, machine->bootarg);

	/*  a2 = pointer to environment strings  */
	cpu->cd.mips.gpr[MIPS_GPR_A2] = (int32_t)0x9fc01800;
	{
		char tmps[50];
		store_32bit_word(cpu, (int32_t)0x9fc01800, 0x9fc01900);
		store_32bit_word(cpu, (int32_t)0x9fc01804, 0x9fc01a00);
		store_32bit_word(cpu, (int32_t)0x9fc01808, 0);

		snprintf(tmps, sizeof(tmps), "memsize=0x%08x",
		    machine->physical_ram_in_mb * 1048576);
		store_string(cpu, (int)0x9fc01900, tmps);
		store_string(cpu, (int)0x9fc01a00,
		    "ethaddr=10:20:30:30:20:10");
	}
}


MACHINE_DEFAULT_CPU(algor)
{
	machine->cpu_name = strdup("RM5200");
}


MACHINE_REGISTER(algor)
{
	MR_DEFAULT(algor, "Algor evaluation board", ARCH_MIPS, MACHINE_ALGOR);

	machine_entry_add_alias(me, "algor");

	machine_entry_add_subtype(me, "P4032", MACHINE_ALGOR_P4032,
	    "p4032", NULL);

	machine_entry_add_subtype(me, "P5064", MACHINE_ALGOR_P5064,
	    "p5064", NULL);
}

